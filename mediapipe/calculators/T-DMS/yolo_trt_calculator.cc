#ifndef MEDIAPIPE_CALCULATORS_T_DMS_YOLO_TRT_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_YOLO_TRT_CALCULATOR_H_

#include <NvInfer.h>
#include <NvOnnxParser.h>
#include <cuda_runtime.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/calculators/T-DMS/yolo_trt_calculator_options.pb.h"

namespace mediapipe {


class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kWARNING)
            std::cout << "[TensorRT] " << msg << std::endl;
    }
} logger;


class YoloTRTCalculator : public CalculatorBase {
private:
    bool verbose_;
    std::string model_onnx_path_;
    std::string model_engine_path_;
    int num_classes_;
    YoloTRTCalculatorOptions::NetworkType network_type_;
    float min_score_thresh_;
    nvinfer1::ICudaEngine* engine = nullptr;
    nvinfer1::IExecutionContext* context = nullptr;

    std::string inputName;
    std::string outputName;

    size_t inSize;
    size_t outSize;

    std::vector<float>* hostInput;
    std::vector<float>* hostOutput;
   
    void* deviceInput = nullptr;
    void* deviceOutput = nullptr;

    cudaStream_t stream;


    std::vector<char> readFileBytes(const std::string &path) {
        std::ifstream ifs(path, std::ios::binary | std::ios::ate);
        if (!ifs.good()) throw std::runtime_error("Cannot open file: " + path);
        size_t size = ifs.tellg();
        std::vector<char> buf(size);
        ifs.seekg(0, std::ios::beg);
        ifs.read(buf.data(), size);
        return buf;
    }

    void writeFileBytes(const std::string &path, const void* data, size_t size) {
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data), size);
        ofs.close();
    }
    
    void buildAndLoadEngine() { // !!! Maybe need some RETCHECK here !!!
        nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(logger);
        if (std::filesystem::exists(model_engine_path_)) {
            std::cout << "📦 Loading engine: " << model_engine_path_ << std::endl;
            auto data = readFileBytes(model_engine_path_);
            engine = runtime->deserializeCudaEngine(data.data(), data.size());
            if (!engine) { std::cerr << "Failed to deserialize engine file.\n"; return; }
        } else {
            std::cout << "🔧 Building engine from ONNX: " << model_onnx_path_ << std::endl;
            nvinfer1::IBuilder* builder = nvinfer1::createInferBuilder(logger);
            auto network = builder->createNetworkV2(0);
            auto parser = nvonnxparser::createParser(*network, logger);
            if (!parser->parseFromFile(model_onnx_path_.c_str(), static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
                std::cerr << "Failed to parse ONNX model!\n"; return;
            }
            nvinfer1::IBuilderConfig* config = builder->createBuilderConfig();
            config->setMemoryPoolLimit(nvinfer1::MemoryPoolType::kWORKSPACE, 1ULL << 30);
            if (builder->platformHasFastFp16()) { // !!! Need check type here !!!
                config->setFlag(nvinfer1::BuilderFlag::kFP16);
                std::cout << "✅ FP16 mode enabled!\n";
            } else {
                std::cout << "⚠️ FP16 not supported on this device.\n";
            }

            auto plan = builder->buildSerializedNetwork(*network, *config);
            if (!plan) { std::cerr << "Engine build failed!\n"; return; }
            engine = runtime->deserializeCudaEngine(plan->data(), plan->size());
            writeFileBytes(model_engine_path_, plan->data(), plan->size());
            std::cout << "✅ Engine serialized & saved: " << model_engine_path_ << std::endl;
        }

        if (!engine) { std::cerr << "No engine available.\n"; return; }

        nvinfer1::IExecutionContext* context = engine->createExecutionContext();
        if (!context) { std::cerr << "Failed to create execution context\n"; return; }
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
        cc->Outputs().Tag("DETECTIONS").Set<std::vector<Detection>>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
        cc->SetOffset(TimestampDiff(0));
        const auto& options = cc->Options<YoloTRTCalculatorOptions>();
        verbose_ = options.verbose();
        model_onnx_path_ = options.model_onnx_path();
        model_engine_path_ = options.model_engine_path();
        num_classes_ = options.num_classes();
        network_type_ = options.network_type();
        min_score_thresh_ = options.min_score_thresh();

        // Load model
        buildAndLoadEngine();
        if (verbose_) {
            LOG(INFO) << "YoloTRTCalculator initialized with model: " << model_onnx_path_;
        }
        // 3) Determine input/output names and sizes for allocating cuda buffers
        int nbIO = engine->getNbIOTensors();
        if (nbIO < 2) {
            std::cerr << "Engine does not have >=2 IO tensors (found " << nbIO << ")\n";
            return absl::OkStatus();;
        }
        inputName = engine->getIOTensorName(0);
        outputName = engine->getIOTensorName(1);
        std::cout << "Input tensor: " << inputName << ", Output tensor: " << outputName << std::endl;

        auto inDims = engine->getTensorShape(inputName.c_str());
        auto outDims = engine->getTensorShape(outputName.c_str());

        inSize = 1;
        for (int i=0;i<inDims.nbDims;++i) {
            inSize *= (inDims.d[i] > 0 ? inDims.d[i] : 1);
        }
        outSize = 1;
        for (int i=0;i<outDims.nbDims;++i) {
            outSize *= (outDims.d[i] > 0 ? outDims.d[i] : 1);
        }

        std::cout << "Engine expects input elements="<<inSize<<", output elements="<<outSize<<"\n";
        
        // 4) Allocate buffers
        hostInput = new std::vector<float>(inSize);
        hostOutput = new std::vector<float>(outSize);
        if (cudaMalloc(&deviceInput, inSize * sizeof(float)) != cudaSuccess) {
            std::cerr << "cudaMalloc failed for input\n"; return absl::OkStatus();
;
        }
        if (cudaMalloc(&deviceOutput, outSize * sizeof(float)) != cudaSuccess) {
            std::cerr << "cudaMalloc failed for output\n"; return absl::OkStatus();
        }

        cudaStreamCreate(&stream);

        return absl::OkStatus();
    }
    absl::Status Process(CalculatorContext* cc) override {
        if (cc->Inputs().Tag("IMAGE").IsEmpty()) {
            return absl::OkStatus();
        }
        const ImageFrame& input_image_frame = cc->Inputs().Tag("IMAGE").Get<ImageFrame>();
        cv::Mat input_mat = formats::MatView(&input_image_frame);
        
        
        input_mat.convertTo(input_mat, CV_32FC3, 1.0f/255.0f);

        int C = 3;
        int H = input_mat.rows, W = input_mat.cols;
        size_t plane = H * W;
        for (int c=0;c<C;++c) {
            float* dst = hostInput->data() + c * plane;
            for (int y=0;y<H;++y) {
                float* rowPtr = (float*)input_mat.ptr<float>(y);
                for (int x=0;x<W;++x)
                    dst[y*W + x] = rowPtr[x*3 + c];
            }
        }

        cudaMemcpyAsync(deviceInput, hostInput->data(), inSize * sizeof(float), cudaMemcpyHostToDevice, stream);
        context->setInputTensorAddress(inputName.c_str(), deviceInput);
        context->setOutputTensorAddress(outputName.c_str(), deviceOutput);
        if (!context->enqueueV3(stream)) {
            std::cerr << "enqueueV3 failed\n";
            return absl::OkStatus();
        }
        cudaMemcpyAsync(hostOutput->data(), deviceOutput, outSize * sizeof(float), cudaMemcpyDeviceToHost, stream);
        cudaStreamSynchronize(stream);

        // === Postprocess mới cho output shape [1,4 + num_classes_, 8400] ===
        std::vector<Detection> detections;
        int detCount = static_cast<int>(outSize / (4 + num_classes_));

        for (int i = 0; i < detCount; ++i) {
            int base = i * (4 + num_classes_);
            float x1_n = (*hostOutput)[base + 0]; // normalized x1 (0..1)
            float y1_n = (*hostOutput)[base + 1]; // normalized y1
            float x2_n = (*hostOutput)[base + 2]; // normalized x2
            float y2_n = (*hostOutput)[base + 3]; // normalized y2

            // find best class and its confidence among 5 classes
            float best_conf = -1.0f;
            int best_cls = -1;
            for (int c = 0; c < num_classes_; ++c) {
                float conf = (*hostOutput)[base + 4 + c];
                if (conf > best_conf) {
                    best_conf = conf;
                    best_cls = c;
                }
            }
            if (best_conf < min_score_thresh_) {
                continue;
            }

            Detection detection;
            detection.set_label_id(0, best_cls);
            detection.set_score(0, best_conf);
            auto* loc = detection.mutable_location_data();
            loc->set_format(LocationData::RELATIVE_BOUNDING_BOX);
            auto* bbox = loc->mutable_relative_bounding_box();
            bbox->set_xmin(x1_n - x2_n / 2);
            bbox->set_ymin(y1_n - y2_n / 2);
            bbox->set_width(x2_n);
            bbox->set_height(y2_n);

            detections.push_back(detection);
        }
        cc->Outputs().Tag("DETECTIONS").Add(new std::vector<Detection>(detections), cc->InputTimestamp());
        return absl::OkStatus();
    }
};

REGISTER_CALCULATOR(YoloTRTCalculator);
}
#endif // MEDIAPIPE_CALCULATORS_T_DMS_YOLO_TRT_CALCULATOR_H_
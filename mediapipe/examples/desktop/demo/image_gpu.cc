#include <cstdlib>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gpu_shared_data_internal.h"
#include "mediapipe/util/resource_util.h"

constexpr char kInputStream[] = "input_image";
constexpr char kOutputStream[] = "output_image";
constexpr char kWindowName[] = "MediaPipe";

 // ...existing includes...

ABSL_FLAG(std::string, calculator_graph_config_file, "", "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_image_path, "", "Path to input image.");
ABSL_FLAG(std::string, output_image_path, "", "Path to save result image.");

absl::Status RunMPPGraph() {
  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
      absl::GetFlag(FLAGS_calculator_graph_config_file),
      &calculator_graph_config_contents));
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  mediapipe::CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  MP_ASSIGN_OR_RETURN(auto gpu_resources, mediapipe::GpuResources::Create());
  MP_RETURN_IF_ERROR(graph.SetGpuResources(std::move(gpu_resources)));
  mediapipe::GlCalculatorHelper gpu_helper;
  gpu_helper.InitializeForTest(graph.GetGpuResources().get());

  // Load input image
  std::string input_path = absl::GetFlag(FLAGS_input_image_path);
  RET_CHECK(!input_path.empty()) << "Input image path is empty!";
  cv::Mat input_bgr = cv::imread(input_path, cv::IMREAD_COLOR);
  RET_CHECK(!input_bgr.empty()) << "Failed to load image: " << input_path;
  cv::Mat input_rgba;
  cv::cvtColor(input_bgr, input_rgba, cv::COLOR_BGR2RGBA);

  // Wrap Mat into an ImageFrame.
  auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
      mediapipe::ImageFormat::SRGBA, input_rgba.cols, input_rgba.rows,
      mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
  cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
  input_rgba.copyTo(input_frame_mat);

  // Start the graph and poller
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                      graph.AddOutputStreamPoller(kOutputStream));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Send image as GPU buffer
  size_t frame_timestamp_us = 0;
  MP_RETURN_IF_ERROR(
      gpu_helper.RunInGlContext([&input_frame, &frame_timestamp_us, &graph, &gpu_helper]() -> absl::Status {
        auto texture = gpu_helper.CreateSourceTexture(*input_frame.get());
        auto gpu_frame = texture.GetFrame<mediapipe::GpuBuffer>();
        glFlush();
        texture.Release();
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            kInputStream, mediapipe::Adopt(gpu_frame.release())
                              .At(mediapipe::Timestamp(frame_timestamp_us))));
        return absl::OkStatus();
      }));

  // Get result
  mediapipe::Packet packet;
  if (!poller.Next(&packet)) {
    return absl::InternalError("No output packet received.");
  }
  std::unique_ptr<mediapipe::ImageFrame> output_frame;

  // Convert GpuBuffer to ImageFrame.
  MP_RETURN_IF_ERROR(gpu_helper.RunInGlContext(
      [&packet, &output_frame, &gpu_helper]() -> absl::Status {
        auto& gpu_frame = packet.Get<mediapipe::GpuBuffer>();
        auto texture = gpu_helper.CreateSourceTexture(gpu_frame);
        output_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormatForGpuBufferFormat(gpu_frame.format()),
            gpu_frame.width(), gpu_frame.height(),
            mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
        gpu_helper.BindFramebuffer(texture);
        const auto info = mediapipe::GlTextureInfoForGpuBufferFormat(
            gpu_frame.format(), 0, gpu_helper.GetGlVersion());
        glReadPixels(0, 0, texture.width(), texture.height(), info.gl_format,
                     info.gl_type, output_frame->MutablePixelData());
        glFlush();
        texture.Release();
        return absl::OkStatus();
      }));

  // Convert to BGR and save
  cv::Mat output_frame_mat = mediapipe::formats::MatView(output_frame.get());
  if (output_frame_mat.channels() == 4)
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGBA2BGR);
  else
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);

  std::string output_path = absl::GetFlag(FLAGS_output_image_path);
  RET_CHECK(!output_path.empty()) << "Output image path is empty!";
  cv::imwrite(output_path, output_frame_mat);

  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  absl::Status run_status = RunMPPGraph();
  if (!run_status.ok()) {
    ABSL_LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    return EXIT_FAILURE;
  } else {
    ABSL_LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}
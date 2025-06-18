#include <cstdlib>
#include <iostream>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/gpu_buffer.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gpu_shared_data_internal.h"
#include "mediapipe/gpu/gl_context.h"
#include "mediapipe/gpu/gl_texture_buffer.h"

constexpr char kInputStream[] = "input_image_gpu";
constexpr char kOutputStream[] = "output_image_gpu";

ABSL_FLAG(std::string, calculator_graph_config_file, "", "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_image_path, "", "Path to input image.");
ABSL_FLAG(std::string, output_image_path, "output_result.jpg", "Path to save output image.");

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

  // Prepare GPU resources.
  ASSIGN_OR_RETURN(auto gpu_resources, mediapipe::GpuResources::Create());
  MP_RETURN_IF_ERROR(graph.SetGpuResources(std::move(gpu_resources)));

  // Load input image
  std::string input_path = absl::GetFlag(FLAGS_input_image_path);
  RET_CHECK(!input_path.empty()) << "Input image path is empty!";
  cv::Mat input_bgr = cv::imread(input_path, cv::IMREAD_COLOR);
  RET_CHECK(!input_bgr.empty()) << "Failed to load image: " << input_path;
  cv::Mat input_rgba;
  cv::cvtColor(input_bgr, input_rgba, cv::COLOR_BGR2RGBA);

  // Convert cv::Mat to GpuBuffer
  auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
      mediapipe::ImageFormat::SRGBA, input_rgba.cols, input_rgba.rows,
      mediapipe::ImageFrame::kDefaultAlignmentBoundary);
  cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
  input_rgba.copyTo(input_frame_mat);

  // Get GL context
  ASSIGN_OR_RETURN(auto gl_context, mediapipe::GlContext::Create());
  MP_RETURN_IF_ERROR(gl_context->Finish());

  // Create GpuBuffer from ImageFrame
  mediapipe::GpuBuffer input_gpu_buffer =
      mediapipe::GpuBuffer::FromImageFrame(*input_frame);

  // Start the graph and poller
  MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                      graph.AddOutputStreamPoller(kOutputStream));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  // Send image as GpuBuffer
  size_t frame_timestamp_us = 0;
  MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
      kInputStream, mediapipe::MakePacket<mediapipe::GpuBuffer>(input_gpu_buffer)
                        .At(mediapipe::Timestamp(frame_timestamp_us))));

  // Get result and save
  mediapipe::Packet packet;
  if (!poller.Next(&packet)) {
    return absl::InternalError("No output packet received.");
  }

  // Get output as GpuBuffer and convert to ImageFrame for saving
  const auto& output_gpu_buffer = packet.Get<mediapipe::GpuBuffer>();
  mediapipe::ImageFrame output_frame;
  MP_RETURN_IF_ERROR(mediapipe::ConvertGpuBufferToImageFrame(output_gpu_buffer, &output_frame));
  cv::Mat output_mat = mediapipe::formats::MatView(&output_frame);

  // Convert to BGR for saving
  cv::Mat output_bgr;
  if (output_mat.channels() == 4)
    cv::cvtColor(output_mat, output_bgr, cv::COLOR_RGBA2BGR);
  else if (output_mat.channels() == 3)
    cv::cvtColor(output_mat, output_bgr, cv::COLOR_RGB2BGR);
  else
    output_bgr = output_mat.clone();

  // Save to file
  std::string output_path = absl::GetFlag(FLAGS_output_image_path);
  cv::imwrite(output_path, output_bgr);
  ABSL_LOG(INFO) << "Saved output image to: " << output_path;

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
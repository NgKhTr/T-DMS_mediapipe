#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/absl_log.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/port/status.h"

constexpr char kInputStream[] = "IMAGE:image_in";
constexpr char kDetectionsStream[] = "DETECTIONS:detections_out";
constexpr char kWindowName[] = "MediaPipe Object Detection";

ABSL_FLAG(std::string, calculator_graph_config_file, "", "Path to CalculatorGraphConfig proto.");
ABSL_FLAG(int, camera_id, 0, "Camera ID to use.");

void DrawDetections(cv::Mat& frame, const std::vector<mediapipe::Detection>& detections) {
    for (const auto& detection : detections) {
        if (detection.location_data().format() != mediapipe::LocationData::RELATIVE_BOUNDING_BOX) continue;
        const auto& bbox = detection.location_data().relative_bounding_box();
        int x = static_cast<int>(bbox.xmin() * frame.cols);
        int y = static_cast<int>(bbox.ymin() * frame.rows);
        int w = static_cast<int>(bbox.width() * frame.cols);
        int h = static_cast<int>(bbox.height() * frame.rows);
        cv::rectangle(frame, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
        if (detection.label_size() > 0) {
            std::string label = detection.label(0);
            float score = detection.score_size() > 0 ? detection.score(0) : 0.0f;
            cv::putText(frame, label + " " + std::to_string(score), cv::Point(x, y - 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        }
    }
}

absl::Status RunMPPGraph() {
    std::string graph_path = absl::GetFlag(FLAGS_calculator_graph_config_file);
    MP_RETURN_IF_ERROR(mediapipe::file::Exists(graph_path));
    std::string graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(graph_path, &graph_config_contents));
    mediapipe::CalculatorGraphConfig config =
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(graph_config_contents);

    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));
    MP_ASSIGN_OR_RETURN(auto poller, graph.AddOutputStreamPoller(kDetectionsStream));
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    cv::VideoCapture cap(absl::GetFlag(FLAGS_camera_id));
    RET_CHECK(cap.isOpened()) << "Failed to open camera.";

    cv::namedWindow(kWindowName, cv::WINDOW_AUTOSIZE);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Convert to mediapipe::Image (CPU)
        cv::Mat frame_rgb;
        cv::cvtColor(frame, frame_rgb, cv::COLOR_BGR2RGB);
        mediapipe::Image mp_image = mediapipe::formats::MatViewToImage(frame_rgb);

        size_t frame_timestamp_us = static_cast<size_t>(cv::getTickCount() / cv::getTickFrequency() * 1e6);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            kInputStream, mediapipe::MakePacket<mediapipe::Image>(mp_image)
                              .At(mediapipe::Timestamp(frame_timestamp_us))));

        mediapipe::Packet packet;
        if (!poller.Next(&packet)) break;

        const auto& detections = packet.Get<std::vector<mediapipe::Detection>>();
        DrawDetections(frame, detections);

        cv::imshow(kWindowName, frame);
        if (cv::waitKey(1) == 27) break; // ESC to exit
    }

    MP_RETURN_IF_ERROR(graph.CloseInputStream("IMAGE:image_in"));
    return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    absl::ParseCommandLine(argc, argv);
    absl::Status run_status = RunMPPGraph();
    if (!run_status.ok()) {
        ABSL_LOG(ERROR) << "Failed to run the graph: " << run_status.message();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
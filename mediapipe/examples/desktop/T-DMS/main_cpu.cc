// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
#include <cstdlib>

#include <chrono>
#include <thread>
#include <sstream>
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
#include "mediapipe/util/resource_util.h"

constexpr char kInputStream[] = "input_frame";
constexpr char kOutputStream[] = "output_frame";
constexpr char kWindowName[] = "T-DMS";

const auto warm_up_phase_duration = std::chrono::seconds(10);
const auto stopped_phase_duration = std::chrono::seconds(2);

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

absl::Status RunMPPGraph() {
    std::string calculator_graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
        absl::GetFlag(FLAGS_calculator_graph_config_file),
        &calculator_graph_config_contents));
    ABSL_LOG(INFO) << "Get calculator graph config contents: "
                   << calculator_graph_config_contents;
    mediapipe::CalculatorGraphConfig config =
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
            calculator_graph_config_contents);

    ABSL_LOG(INFO) << "Initialize the calculator graph.";
    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    // const int output_width = 1280, output_height = 720;
    const int input_width = 1920, input_height = 1080;
    const int output_width = 1920, output_height = 1080;
    // const int output_width = 10000, output_height = 10720;

    ABSL_LOG(INFO) << "Initialize the camera or load the video.";
    cv::VideoCapture capture;
    const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();
    if (load_video) {
        capture.open(absl::GetFlag(FLAGS_input_video_path));
    } else {
        std::ostringstream pipeline_oss;
        pipeline_oss << "v4l2src device=/dev/video0 ! image/jpeg,width=" << input_width
                      << ",height=" << input_height << ",framerate=30/1 ! jpegdec ! videoconvert ! appsink";
        std::string pipeline = pipeline_oss.str();
        capture.open(pipeline, cv::CAP_GSTREAMER);
    }
    RET_CHECK(capture.isOpened());

    cv::VideoWriter writer;
    const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();

    int w = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
	int h = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
	double fps = capture.get(cv::CAP_PROP_FPS);

    double scale = -1;
    if (w > output_width || h > output_height) {
        scale = std::min(
            static_cast<double>(output_width) / w,
            static_cast<double>(output_height) / h);
        w = static_cast<int>(w * scale);
        h = static_cast<int>(h * scale);
    }

	ABSL_LOG(INFO) << "Opened at " << w << "x" << h << " @" << fps << " FPS\n";
    if (save_video) {
        ABSL_LOG(INFO) << "Prepare video writer.";
        writer.open(absl::GetFlag(FLAGS_output_video_path),
                    mediapipe::fourcc('a', 'v', 'c', '1'),
                    fps, cv::Size(w, h));
        RET_CHECK(writer.isOpened());
    } else {
        // cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
        cv::namedWindow(kWindowName, cv::WINDOW_NORMAL);
        cv::setWindowProperty(kWindowName, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    }

    std::shared_ptr<cv::Mat> latest_output_frame = std::make_shared<cv::Mat>();
    std::mutex frame_mutex;
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        *latest_output_frame = cv::Mat(h, w, CV_8UC3, cv::Scalar(255, 20, 20));
    }
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream(
        kOutputStream,
        [latest_output_frame, &frame_mutex](const mediapipe::Packet& packet) -> absl::Status {
            auto& output_frame = packet.Get<mediapipe::ImageFrame>();
            cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);

            if (output_frame_mat.channels() == 4) {
				cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGBA2BGR);
            } else {
                cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
            }

            {
                std::lock_guard<std::mutex> lock(frame_mutex);
                *latest_output_frame = output_frame_mat.clone();
            }
            return absl::OkStatus();
        }));

    MP_RETURN_IF_ERROR(graph.StartRun({}));

    ABSL_LOG(INFO) << "Start grabbing and processing frames.";

    int ms_delay = static_cast<int>(1000.0 / fps); // milliseconds per frame
	const auto frame_duration = std::chrono::milliseconds(ms_delay);
    auto next_frame_time = std::chrono::steady_clock::now();
	auto end_phase_time = next_frame_time + warm_up_phase_duration;
 
	enum class Phase {
		WARM_UP,
		RUNNING,
		STOPPED
	};
	Phase current_phase = Phase::WARM_UP;
    while (true) {
        // Capture opencv camera or video frame.
        cv::Mat frame_raw;
		if (current_phase == Phase::WARM_UP) {
			frame_raw = cv::Mat(h, w, CV_8UC3, cv::Scalar(20, 20, 20));
		} else if (current_phase == Phase::RUNNING) {
			capture >> frame_raw;
			if (frame_raw.empty()) {
				// if (!load_video) {
				// 	// ABSL_LOG(INFO) << "Ignore empty frames from camera.";
				// 	continue;
				// }
				current_phase = Phase::STOPPED;
				end_phase_time = next_frame_time + stopped_phase_duration;
                ABSL_LOG(INFO) << "Change phase to STOPPED.";
				frame_raw = cv::Mat(h, w, CV_8UC3, cv::Scalar(20, 20, 20));
			} else if (scale > 0) {
                cv::resize(frame_raw, frame_raw, cv::Size(), scale, scale, cv::INTER_AREA);
            }
		} else if (current_phase == Phase::STOPPED) {
			frame_raw = cv::Mat(h, w, CV_8UC3, cv::Scalar(20, 20, 20));
		}
		cv::Mat frame;
		cv::cvtColor(frame_raw, frame, cv::COLOR_BGR2RGB);
		if (!load_video) {
			cv::flip(frame, frame, /*flipcode=HORIZONTAL*/ 1);
		}

        // Wrap Mat into an ImageFrame.
        auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormat::SRGB, frame.cols, frame.rows,
            mediapipe::ImageFrame::kDefaultAlignmentBoundary);
        cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
        frame.copyTo(input_frame_mat);

        // Send image packet into the graph.
        size_t frame_timestamp_us =
            (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            kInputStream, mediapipe::Adopt(input_frame.release())
                              .At(mediapipe::Timestamp(frame_timestamp_us))));

        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (!latest_output_frame->empty()) {
                if (save_video && (current_phase == Phase::RUNNING || current_phase == Phase::STOPPED)) {
                    writer.write(*latest_output_frame);
                }
                // else {
                    cv::imshow(kWindowName, *latest_output_frame);
                    int pressed_key = cv::waitKey(5);
                    if (pressed_key >= 0 && pressed_key != 255) {
                        current_phase = Phase::STOPPED;
                        end_phase_time = next_frame_time + stopped_phase_duration;
                        ABSL_LOG(INFO) << "Change phase to STOPPED.";
                    }
                // }
            }
        }

        next_frame_time += frame_duration;
        std::this_thread::sleep_until(next_frame_time);
		if (current_phase == Phase::WARM_UP && next_frame_time >= end_phase_time) {
            current_phase = Phase::RUNNING;
            ABSL_LOG(INFO) << "Change phase to RUNNING.";
		} else if (current_phase == Phase::STOPPED && next_frame_time >= end_phase_time) {
			break;
		}
    }

    ABSL_LOG(INFO) << "Shutting down.";
    if (writer.isOpened()) {
        writer.release();
    }
    if (capture.isOpened()) {
        capture.release();
    }
    ABSL_LOG(INFO) << "Shut down.";
    MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
    return graph.WaitUntilDone();
}

int main(int argc, char **argv)
{
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

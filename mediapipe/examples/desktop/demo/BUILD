cc_binary(
    name = "app",
    srcs = ["app.cc"],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/calculators/calculators:custom_max_calculator"
    ],
    data = ["graph.pbtxt"],
)

cc_binary(
    name = "play_audio",
    srcs = ["play_audio.cc"],
    deps = [":sfml"],
)

cc_library(
    name = "sfml",
    includes = ["/usr/include"],
    linkopts = [
        "-lsfml-audio",
        "-lsfml-system",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "object_detection_infer_calculator",
    srcs = ["object_detection_infer_calculator.cc"],
    data = [
        # Object detection
        # "//home/nextwave/Desktop/T-DMS/efficientdet_lite0.tflite", # !!!
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:resource_util",
        "//mediapipe/calculators/core:constant_side_packet_calculator",

        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
        # Face
        "//mediapipe/modules/face_landmark:face_landmark_front_gpu", # FaceLandmarkFrontCpu
        # For iris
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/T-DMS/subgraphs:iris_landmarks_subgraph",

        "//mediapipe/calculators/T-DMS:eye_contour_landmarks_to_render_data_calculator", # Render for dev

        "//mediapipe/graphs/T-DMS/subgraphs:iris_and_face_landmarks_render_subgraph",
        # Object detection
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_render_subgraph",
        # Hand
        "//mediapipe/modules/palm_detection:palm_detection_gpu",
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_gpu",
        "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_gpu",

        "//mediapipe/calculators/T-DMS:hand_landmarks_to_render_data_calculator", # Hand holding phone detection
        # Latency
        "//mediapipe/calculators/T-DMS:latency_calculator",
        "//mediapipe/calculators/T-DMS:start_latency_calculator",

        "//mediapipe/framework/formats:tensor", # For mediapipe::Tensor

        "//mediapipe/calculators/core:split_vector_calculator", # For split vector of tensors

        "//mediapipe/calculators/T-DMS:anchor_csv_side_packet_calculator"
    ]
)

cc_binary(
    name = "object_detection_detector",
    srcs = ["object_detection_detector.cc"],
    data = [
        # Object detection
        # "//home/nextwave/Desktop/T-DMS/efficientdet_lite0.tflite", # !!!
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:resource_util",
        "//mediapipe/calculators/core:constant_side_packet_calculator",

        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",

        "//mediapipe/framework/formats:tensor", # For mediapipe::Tensor

        "//mediapipe/calculators/core:split_vector_calculator", # For split vector of tensors
    ]
)
cc_binary(
    name = "image_cpu",
    srcs = ["image_cpu.cc"],
    data = [
        # Object detection
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
        
        "//mediapipe/models:labelmap.txt",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms_anchors.csv",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms.tflite",
        # Face
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        # Hand
        "//mediapipe/modules/hand_landmark:hand_landmark_full.tflite",
        "//mediapipe/modules/palm_detection:palm_detection_full.tflite",
        # Iris: also need face
        "//mediapipe/modules/iris_landmark:iris_landmark.tflite",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:resource_util",
        "//mediapipe/calculators/core:constant_side_packet_calculator",

        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
        # Face
        "//mediapipe/modules/face_landmark:face_landmark_front_cpu", # FaceLandmarkFrontCpu
        "//mediapipe/calculators/demo:face_landmarks_to_render_index_data_calculator", # Hand holding phone detection
        # For yawn
        "//mediapipe/calculators/T-DMS:yawn_calculator",
        # For iris
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/T-DMS/subgraphs:iris_landmarks_subgraph",

        "//mediapipe/calculators/demo:eye_contour_landmarks_to_render_data_calculator", # Render for dev

        "//mediapipe/graphs/T-DMS/subgraphs:iris_and_face_landmarks_render_subgraph",
        # Object detection
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_ssd_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_render_subgraph",
        "//mediapipe/calculators/core:split_vector_calculator", # For split vector of tensors
        "//mediapipe/calculators/T-DMS:anchor_csv_side_packet_calculator",
        # Hand
        "//mediapipe/modules/palm_detection:palm_detection_gpu",
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_gpu",
        "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_gpu",

        "//mediapipe/calculators/demo:hand_landmarks_to_render_data_calculator", # Hand holding phone detection
        # Latency
        "//mediapipe/calculators/T-DMS:latency_calculator",
        "//mediapipe/calculators/T-DMS:start_latency_calculator",
    ]
)
cc_binary(
    name = "image_gpu",
    srcs = ["image_gpu.cc"],
    data = [
        # Object detection
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
        
        "//mediapipe/models:labelmap.txt",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms_anchors.csv",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms.tflite",
        # Face
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        # Hand
        "//mediapipe/modules/hand_landmark:hand_landmark_full.tflite",
        "//mediapipe/modules/palm_detection:palm_detection_full.tflite",
        # Iris: also need face
        "//mediapipe/modules/iris_landmark:iris_landmark.tflite",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:resource_util",
        "//mediapipe/calculators/core:constant_side_packet_calculator",

        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
        # Face
        "//mediapipe/modules/face_landmark:face_landmark_front_gpu", # FaceLandmarkFrontCpu
        # For iris
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/T-DMS/subgraphs:iris_landmarks_subgraph",

        "//mediapipe/calculators/demo:eye_contour_landmarks_to_render_data_calculator", # Render for dev
        "//mediapipe/calculators/demo:iris_landmarks_to_render_data_calculator", # Render for dev

        "//mediapipe/graphs/T-DMS/subgraphs:iris_and_face_landmarks_render_subgraph",
        # Object detection
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_render_subgraph",
        "//mediapipe/calculators/core:split_vector_calculator", # For split vector of tensors
        "//mediapipe/calculators/T-DMS:anchor_csv_side_packet_calculator",
        # Hand
        "//mediapipe/modules/palm_detection:palm_detection_gpu",
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_gpu",
        "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_gpu",

        "//mediapipe/calculators/demo:hand_landmarks_to_render_data_calculator", # Hand holding phone detection
        # Latency
        "//mediapipe/calculators/T-DMS:latency_calculator",
        "//mediapipe/calculators/T-DMS:start_latency_calculator",

        "//mediapipe/gpu:gl_calculator_helper", # For #include "mediapipe/gpu/gl_calculator_helper.h"
    ]
)
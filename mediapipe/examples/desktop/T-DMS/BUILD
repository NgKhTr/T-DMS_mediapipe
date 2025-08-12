cc_binary(
    name = "main",
    srcs = ["main_gpu.cc"],
    data = [
        # Object detection
            # efficientdet_lite0_v1
        "//mediapipe/models:labelmap.txt",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms_anchors.csv",
        "//mediapipe/models:efficientdet_lite0_fp16_no_nms.tflite",
            # SSD: deprecated
        # "//mediapipe/models:ssdlite_object_detection.tflite",
        # "//mediapipe/models:ssdlite_object_detection_labelmap.txt",

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
        # Convert
        "//mediapipe/gpu:gpu_buffer_to_image_frame_calculator",
        "//mediapipe/gpu:image_frame_to_gpu_buffer_calculator",
        "//mediapipe/calculators/T-DMS:image_frame_to_base64_string_calculator",
        
        # Face
        "//mediapipe/modules/face_landmark:face_landmark_front_gpu", # FaceLandmarkFrontCpu
        # For yawn
        "//mediapipe/calculators/T-DMS:yawn_calculator",
        # For iris
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/T-DMS/subgraphs:iris_landmarks_subgraph",

        "//mediapipe/calculators/T-DMS:all_eyes_closed_calculator",

        "//mediapipe/graphs/T-DMS/subgraphs:iris_and_face_landmarks_render_subgraph",

        "//mediapipe/calculators/T-DMS:gaze_calculator",
        # Object detection
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_ssd_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_efficient_det_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_mobilenet_v2_i320_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_render_subgraph",
        "//mediapipe/calculators/core:split_vector_calculator", # For split vector of tensors
        "//mediapipe/calculators/T-DMS:anchor_side_packet_calculator",
        # Hand
        "//mediapipe/modules/palm_detection:palm_detection_gpu",
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_gpu",
        # "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_gpu",
        "//mediapipe/graphs/T-DMS/subgraphs:hand_landmarks_render_subgraph",
        # SOS hand sign calculator
        "//mediapipe/calculators/T-DMS:sos_hand_sign_calculator",

        # Hand hold detecion
        "//mediapipe/calculators/T-DMS:hand_hold_object_calculator",

        # Threaten calculator
        "//mediapipe/calculators/T-DMS:threaten_calculator",

        # Latency
        "//mediapipe/calculators/T-DMS:latency_calculator", 
        "//mediapipe/calculators/T-DMS:start_latency_calculator",
        # Flow
        "//mediapipe/calculators/T-DMS:condition_calculator",
        # Alert
        "//mediapipe/calculators/T-DMS:audio_alert_calculator",
        "//mediapipe/calculators/T-DMS:cooling_calculator",
        "//mediapipe/calculators/T-DMS:http_post_calculator",
        "//mediapipe/calculators/T-DMS:mqtt_calculator",

        # Check flow: for debugging
        "//mediapipe/calculators/demo:check_flow_calculator",
    ]
)

cc_binary(
    name = "main_gpu_copy",
    srcs = ["main_gpu_copy.cc"],
    data = [
        # Object detection
        "//mediapipe/models:ssdlite_object_detection.tflite",
        "//mediapipe/models:ssdlite_object_detection_labelmap.txt",
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

        "//mediapipe/calculators/T-DMS:all_eyes_closed_calculator",

        "//mediapipe/graphs/T-DMS/subgraphs:iris_and_face_landmarks_render_subgraph",
        # Object detection
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_subgraph",
        "//mediapipe/graphs/T-DMS/subgraphs:object_detection_render_subgraph",
        # Hand
        "//mediapipe/modules/palm_detection:palm_detection_gpu",
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_gpu",
        "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_gpu",
        
        # Hand holding phone detecion
        "//mediapipe/calculators/T-DMS:hand_holding_phone_calculator",

        # Latency
        "//mediapipe/calculators/T-DMS:latency_calculator",
        "//mediapipe/calculators/T-DMS:start_latency_calculator",

        # Alert
        "//mediapipe/calculators/T-DMS:audio_alert_calculator",

        # Check flow: for debugging
        "//mediapipe/calculators/demo_calculators:check_flow_calculator",
    ]
)

cc_binary(
    name = "main_cpu",
    srcs = ["main_cpu.cc"],
    data = [
        "//mediapipe/modules/hand_landmark:hand_landmark_full.tflite",
        "//mediapipe/modules/palm_detection:palm_detection_full.tflite",
        # Iris
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
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
        "//mediapipe/modules/hand_landmark:hand_landmark_tracking_cpu",
        "//mediapipe/graphs/hand_tracking/subgraphs:hand_renderer_cpu",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
        # For iris
        "//mediapipe/modules/face_landmark:face_landmark_front_cpu", # FaceLandmarkFrontCpu
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/modules/iris_landmark:iris_landmark_left_and_right_cpu", # IrisLandmarkLeftAndRightCpu
        "//mediapipe/calculators/core:concatenate_proto_list_calculator", #ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/iris_tracking/calculators:update_face_landmarks_calculator", #UpdateFaceLandmarksCalculator
        "//mediapipe/graphs/iris_tracking/subgraphs:iris_renderer_cpu", # IrisRendererCpu
        "//mediapipe/calculators/core:pass_through_calculator", # PassThroughCalculator for image input
    
    ],
)


cc_binary(
    name = "demo_fps",
    srcs = ["main_gpu.cc"],
    data = [
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
    
        "//mediapipe/calculators/core:split_proto_list_calculator", # SplitNormalizedLandmarkListCalculator, ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/calculators/core:concatenate_proto_list_calculator", #ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/examples/desktop:simple_run_graph_main",
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators",

        "//mediapipe/calculators/T-DMS:latency_calculator",
        "//mediapipe/calculators/T-DMS:start_latency_calculator",
    ],

)


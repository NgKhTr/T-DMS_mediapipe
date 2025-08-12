load("//mediapipe/framework/tool:mediapipe_graph.bzl", "mediapipe_simple_subgraph")
package(default_visibility = ["//visibility:public"])

mediapipe_simple_subgraph(
    name = "object_detection_efficient_det_subgraph",
    graph = "object_detection_efficient_det.pbtxt",
    register_as = "ObjectDetectionEfficientDetSubgraph",
    deps = [
        "//mediapipe/calculators/T-DMS:anchor_side_packet_calculator",
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For ImageTransformationCalculatorOptions
    ],
)
mediapipe_simple_subgraph(
    name = "object_detection_mobilenet_v2_subgraph",
    graph = "object_detection_mobilenet_v2.pbtxt",
    register_as = "ObjectDetectionMobilenetV2Subgraph",
    deps = [
        "//mediapipe/calculators/T-DMS:anchor_side_packet_calculator",
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For ImageTransformationCalculatorOptions
    ],
)
mediapipe_simple_subgraph(
    name = "object_detection_ssd_subgraph",
    graph = "object_detection_ssd.pbtxt",
    register_as = "ObjectDetectionSsdSubgraph",
    deps = [
        "//mediapipe/calculators/T-DMS:anchor_csv_side_packet_calculator",
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For ImageTransformationCalculatorOptions
    ],
)

mediapipe_simple_subgraph(
    name = "object_detection_render_subgraph",
    graph = "object_detection_render.pbtxt",
    register_as = "ObjectDetectionRenderSubgraph",
    deps = [
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For DetectionsToRenderDataCalculatorOptions"
    ],
)

mediapipe_simple_subgraph(
    name = "iris_landmarks_subgraph",
    graph = "iris_landmarks.pbtxt",
    register_as = "IrisLandmarksSubgraph",
    deps = [
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For "
        "//mediapipe/modules/iris_landmark:iris_landmark_left_and_right_gpu"
    ]
)
mediapipe_simple_subgraph(
    name = "iris_and_face_landmarks_render_subgraph",
    graph = "iris_and_face_landmarks_render.pbtxt",
    register_as = "IrisAndFaceLandmarkRenderSubgraph",
    deps = [
        "//mediapipe/graphs/object_detection:desktop_tflite_calculators", # For DetectionsToRenderDataCalculatorOptions"
        "//mediapipe/calculators/core:concatenate_proto_list_calculator", #ConcatenateNormalizedLandmarkListCalculator
        "//mediapipe/graphs/iris_tracking/calculators:update_face_landmarks_calculator", #UpdateFaceLandmarksCalculator
        "//mediapipe/graphs/iris_tracking/subgraphs:iris_and_depth_renderer_gpu", # IrisAndDepthRendererGpu
    ]
)
mediapipe_simple_subgraph(
    name = "hand_landmarks_render_subgraph",
    graph = "hand_landmarks_render.pbtxt",
    register_as = "HandLandmarksRenderSubgraph",
    deps = [
        "//mediapipe/calculators/core:begin_loop_calculator", # For BeginLoopNormalizedLandmarkListVectorCalculator
        "//mediapipe/calculators/util:landmarks_to_render_data_calculator", # For LandmarksToRenderDataCalculator
        "//mediapipe/calculators/core:end_loop_calculator",   # For EndLoopRenderDataCalculator
        "//mediapipe/calculators/util:annotation_overlay_calculator", # For AnnotationOverlayCalculator
    ]
)
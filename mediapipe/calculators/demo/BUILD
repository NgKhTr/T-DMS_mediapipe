package(default_visibility = ["//visibility:public"])

cc_library(
    name = "custom_max_calculator",
    srcs = [
        "max_calculator.cc",
    ],
    hdrs = [
        "max_calculator.h",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
    ],
    alwayslink = 1, 
    # must add
)

cc_library(
    name = "check_flow_calculator",
    srcs = [
        "check_flow_calculator.cc",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:tensor",
        "@org_tensorflow//tensorflow/lite:framework", # for tensorflow interpreter TFlitTensor
    ],
    alwayslink = 1, 
    # must add
)

cc_library(
    name = "eye_contour_landmarks_to_render_data_calculator",
    srcs = [
        "eye_contour_landmarks_to_render_data_calculator.cc",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util:render_data_cc_proto", # for render_data.pb.h
    ],
    alwayslink = 1, 
    # must add
)

cc_library(
    name = "hand_landmarks_to_render_data_calculator",
    srcs = [
        "hand_landmarks_to_render_data_calculator.cc",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util:render_data_cc_proto", # for render_data.pb.h
    ],
    alwayslink = 1, 
    # must add
)

cc_library(
    name = "face_landmarks_to_render_index_data_calculator",
    srcs = [
        "face_landmarks_to_render_index_data_calculator.cc",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util:render_data_cc_proto", # for render_data.pb.h
    ],
    alwayslink = 1, 
    # must add
)

cc_library(
    name = "iris_landmarks_to_render_data_calculator",
    srcs = [
        "iris_landmarks_to_render_data_calculator.cc",
    ],
    deps = [
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util:render_data_cc_proto", # for render_data.pb.h
    ],
    alwayslink = 1, 
    # must add
)
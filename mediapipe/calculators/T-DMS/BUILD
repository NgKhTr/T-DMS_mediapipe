load("//mediapipe/framework/port:build_config.bzl", "mediapipe_cc_proto_library")

package(default_visibility = ["//visibility:public"])


cc_library(
    name = "start_latency_calculator",
    srcs = [
        "start_latency_calculator.cc",
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

mediapipe_cc_proto_library(
    name = "latency_calculator_options",
    srcs = ["latency_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_library(
    name = "latency_calculator",
    srcs = [
        "latency_calculator.cc",
    ],
    deps = [
        ":latency_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
    ],
    alwayslink = 1, 
    # must add
)

mediapipe_cc_proto_library(
    name = "all_eyes_closed_calculator_options",
    srcs = ["all_eyes_closed_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_library(
    name = "all_eyes_closed_calculator",
    srcs = [
        "all_eyes_closed_calculator.cc",
    ],
    deps = [
        ":all_eyes_closed_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util/T-DMS:duration_process",
    ],
    alwayslink = 1, 
    # must add
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

mediapipe_cc_proto_library(
    name = "audio_alert_calculator_options",
    srcs = ["audio_alert_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)


cc_library(
    name = "audio_alert_calculator",
    srcs = [
        "audio_alert_calculator.cc",
    ],
    deps = [
        ":audio_alert_calculator_options",
        ":sfml",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
    ],
    alwayslink = 1, 
    # must add
)

mediapipe_cc_proto_library(
    name = "sos_hand_sign_calculator_options",
    srcs = ["sos_hand_sign_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

cc_library(
    name = "sos_hand_sign_calculator",
    srcs = [
        "sos_hand_sign_calculator.cc",
    ],
    deps = [
        ":sos_hand_sign_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
    ],
    alwayslink = 1, 
)

mediapipe_cc_proto_library(
    name = "hand_hold_object_calculator_options",
    srcs = ["hand_hold_object_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)
cc_library(
    name = "hand_hold_object_calculator",
    srcs = [
        "hand_hold_object_calculator.cc",
    ],
    deps = [
        ":hand_hold_object_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/framework/formats:detection_cc_proto",
        "//mediapipe/util/T-DMS:duration_process",
    ],
    alwayslink = 1, 
)

mediapipe_cc_proto_library(
    name = "anchor_csv_side_packet_calculator_options",
    srcs = ["anchor_csv_side_packet_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)
cc_library(
    name = "anchor_csv_side_packet_calculator",
    srcs = [
        "anchor_csv_side_packet_calculator.cc",
    ],
    deps = [
        ":anchor_csv_side_packet_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats/object_detection:anchor_cc_proto", # For "mediapipe/calculators/tflite/anchor.pb.h"
        "//mediapipe/util:resource_util", # For #include "mediapipe/util/resource_util.h"
    ],
    alwayslink = 1,
)

mediapipe_cc_proto_library(
    name = "yawn_calculator_options",
    srcs = ["yawn_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)
cc_library(
    name = "yawn_calculator",
    srcs = [
        "yawn_calculator.cc",
    ],
    deps = [
        ":yawn_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/util/T-DMS:duration_process",
    ],
    alwayslink = 1, 
)

mediapipe_cc_proto_library(
    name = "threaten_calculator_options",
    srcs = ["threaten_calculator_options.proto"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)
cc_library(
    name = "threaten_calculator",
    srcs = [
        "threaten_calculator.cc",
    ],
    deps = [
        ":threaten_calculator_options",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework/formats:landmark_cc_proto", # for landmark.pb.h
        "//mediapipe/framework/formats:detection_cc_proto",
    ],
    alwayslink = 1,
)
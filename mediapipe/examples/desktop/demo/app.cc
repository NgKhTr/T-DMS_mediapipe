#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/calculators/demo_calculators/max_calculator.h"

absl::Status RunMPP() {
    std::string graphFilePath = R"(mediapipe/examples/desktop/demo_app/demo_graph.pbtxt)";
    std::string graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(graphFilePath, &graph_config_contents));

    mediapipe::CalculatorGraphConfig config = 
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(graph_config_contents);
    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    auto cb = [](const mediapipe::Packet &packet)->mediapipe::Status{
        std::cout << packet.Timestamp() << ": RECEIVED " << packet.Get<double>() << std::endl;
        return mediapipe::OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    // Send packets
    for (int i = 0; i < 5; ++i) {
        double v0 = i * 1.0;
        double v1 = 10.0 - i;
        // std::cout << "Iter: " << i << " | v0: " << v0 << " | v1: " << v1 << std::endl;
        mediapipe::Packet pIn0 = mediapipe::MakePacket<double>(v0).At(mediapipe::Timestamp(i)),
            pIn1 = mediapipe::MakePacket<double>(v1).At(mediapipe::Timestamp(i));
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            "in0", pIn0));

        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            "in1", pIn1));
    }


    MP_RETURN_IF_ERROR(graph.CloseInputStream("in0")); // say to graph no more packets
    MP_RETURN_IF_ERROR(graph.CloseInputStream("in1")); // say to graph no more packets

    // Wait for graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());


    return absl::OkStatus();
}

int main(int argc, char** argv) {
    absl::Status status = RunMPP();
    if (!status.ok()) {
        std::cerr << "Failed: " << status.message() << std::endl;
        return 1;
    }
    return 0;
}
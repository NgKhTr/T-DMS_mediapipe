sleep 5
cd /home/nextwave/Desktop/T-DMS/T-DMS_mediapipe
pactl set-default-sink alsa_output.usb-Generic_USB2.0_Device_20121120222016-00.analog-stereo
sleep 5
GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/T-DMS/main_cpu --calculator_graph_config_file=mediapipe/graphs/T-DMS/main.pbtxt
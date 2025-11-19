# T-DMS
## Install
### gcc, g++:
- Version: > 8.x. => get ver 11
- Install:
```bash
sudo apt install g++-11
sudo apt install gcc-11
```
### cmake
```bash
sudo snap install cmake --classic
```
### build-essential
```bash
sudo apt install -y build-essential
```
### pkg-config
```bash
sudo apt install -y pkg-config
```
### libv4l
```bash
sudo apt install -y libv4l-dev v4l-utils
```
### python3-dev
```bash
sudo apt install -y python3-dev
```
### python3-numpy
```bash
sudo apt install -y python3-numpy
```
### libjpeg-dev
```bash
sudo apt install -y libjpeg-dev
```
### libpng-dev
```bash
sudo apt install -y libpng-dev
```
### libtiff-dev
```bash
sudo apt install -y libtiff-dev
```
### libdc1394-22-dev
```bash
sudo apt install -y libdc1394-22-dev
```
### FFmpeg:
#### libavcodec-dev
```bash
sudo apt install -y libavcodec-dev
```
#### libavformat-dev
```bash
sudo apt install -y libavformat-dev
```
#### libavutil-dev
```bash
sudo apt install -y libavutil-dev
```
#### libswscale-dev
```bash
sudo apt install -y libswscale-dev
```
#### Install 
```bash
sudo apt install ffmpeg
```
### Gstreamer
```bash
sudo apt install -y \
libgstreamer1.0-dev \
libgstreamer-plugins-base1.0-dev \
gstreamer1.0-plugins-base \
gstreamer1.0-plugins-good \
gstreamer1.0-plugins-bad \
gstreamer1.0-plugins-ugly \
gstreamer1.0-libav \
gstreamer1.0-tools
```
### gtk
```bash
sudo apt install -y libgtk-3-dev libcanberra-gtk3-module
```
### TTB for intel chip arch: libtbb12 for Ubuntu 20.04 => 24.04
> Now turn off because of opencv build error
```bash
	sudo apt install -y libtbb-dev libtbb12
```
### OpenCV
> Ignore infor from document: https://ai.google.dev/edge/mediapipe/framework/getting_started/install

> Get version: 4.9.0
#### opencv_contrib
- Need to build with https://github.com/opencv/opencv_contrib
```bash
wget -O opencv_contrib_4.9.0.zip https://github.com/opencv/opencv_contrib/archive/refs/tags/4.9.0.zip
unzip opencv_contrib_4.9.0.zip && rm opencv_contrib_4.9.0.zip
```
#### opencv
```bash
wget -O opencv_4.9.0.zip https://github.com/opencv/opencv/archive/refs/tags/4.9.0.zip
unzip opencv_4.9.0.zip && rm opencv_4.9.0.zip
```	
#### Build
```bash
# Remove prev build if exist
rm -rf build

# Create build directory
mkdir -p build && cd build
	 
# Configure
## for NUC 13
cmake -D CMAKE_BUILD_TYPE=RELEASE \
	-D CMAKE_INSTALL_PREFIX=/usr/local \
	-D OPENCV_EXTRA_MODULES_PATH=/home/ubuntu/Desktop/OpenCV_Build/opencv_contrib-4.9.0/modules \ # config path to opencv_contrib
	-D OPENCV_GENERATE_PKGCONFIG=ON \
	-D WITH_GTK=ON \
	-D WITH_CUDA=OFF \
	-D WITH_CUDNN=OFF \
	-D WITH_GSTREAMER=ON \
	-D WITH_FFMPEG=ON \
	-D WITH_LIBV4L=ON \
	-D OPENCV_DNN_CUDA=OFF \
	-D ENABLE_NEON=OFF \
	-D ENABLE_AVX2=ON \
	-D ENABLE_AVX512=ON \
	-D CPU_BASELINE=AVX2,AVX512 \
	-D WITH_TBB=OFF \
	-D WITH_OPENMP=ON \
	# -D WITH_TTB=ON \
	-D BUILD_opencv_python3=ON \
	-D BUILD_opencv_python2=OFF \
	-D PYTHON3_EXECUTABLE=/usr/bin/python3.10 \
	-D BUILD_EXAMPLES=OFF \
	-D BUILD_TESTS=OFF \
	-D BUILD_PERF_TESTS=OFF \
	../opencv-4.9.0
```
- If meet that error:
```
ubuntu@ubuntu-NUC34T:~/Desktop/OpenCV_Build/build$ cmake --build . [ 0%] Generate opencv4.pc CMake Error at /home/ubuntu/Desktop/OpenCV_Build/opencv-4.1.2/cmake/OpenCVGenPkgconfig.cmake:113 (cmake_minimum_required): Compatibility with CMake < 3.5 has been removed from CMake. Update the VERSION argument <min> value. Or, use the <min>...<max> syntax to tell CMake that the project requires at least <min> but has been updated to work with policies introduced by <max> or earlier. Or, add -DCMAKE_POLICY_VERSION_MINIMUM=3.5 to try configuring anyway. gmake[2]: *** [CMakeFiles/gen-pkgconfig.dir/build.make:72: unix-install/opencv4.pc] Error 1 gmake[1]: *** [CMakeFiles/Makefile2:4416: CMakeFiles/gen-pkgconfig.dir/all] Error 2 gmake: *** [Makefile:166: all]
```
- Adjust in `opencv-4.9.0/cmake/OpenCVGenPkgconfig.cmake:113`: `cmake_minimum_required(VERSION 2.8.12.2)` to `cmake_minimum_required(VERSION 3.5)`
```bash
make -j$(nproc)
```

#### Install
```bash
sudo make install -j$(nproc)
```
#### Validate
```bash
opencv_version
```

```python
import cv2
print(cv2.getBuildInformation())
```
#### Remove if wanna to build again with other config
```bash
# Delete installed header, bin
sudo rm -rf /usr/local/lib/libopencv*
sudo rm -rf /usr/local/include/opencv*
sudo rm -rf /usr/local/share/opencv*
sudo rm -rf /usr/local/bin/opencv*
sudo rm -rf /usr/local/lib/python3.10/dist-packages/cv2
sudo rm -f /usr/local/lib/pkgconfig/opencv*.pc

# Update linker cache
sudo ldconfig
```
### Bazel
- Get bazellisk
```bash
cd /usr/local/bin/

# for amd arch:
sudo wget -O bazel https://github.com/bazelbuild/bazelisk/releases/download/v1.27.0/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel
```
### nlohmann
```bash
sudo apt-get install -y nlohmann-json3-dev
```
### sfml
```bash
sudo apt-get install -y libsfml-dev
```
### curl
```bash
sudo apt-get install -y libcurl4-openssl-dev
```
### Paho mqtt
```bash
sudo apt-get install -y libpaho-mqttpp3-1 
sudo apt-get install -y libpaho-mqttpp-dev 
sudo apt-get install -y libpaho-mqtt1.3
sudo apt-get install -y libpaho-mqtt-dev
```
## Check 
```bash
apt list --installed | grep libavutil-dev
```
## Run

```bash
# For NUC
bazel build -c opt \
	--copt -DMESA_EGL_NO_X11_HEADERS \
	--copt -DEGL_NO_X11 \
	--copt -DNDEBUG \
	--define xnn_enable_avx512amx=false \
	--define xnn_enable_avx512fp16=false \
	--define xnn_enable_avxvnniint8=false \
	--define xnn_enable_avxvnni=false \
	mediapipe/examples/desktop/T-DMS:main_cpu
```
  
  
  



	
		
cmake_minimum_required(VERSION 3.16.0)
project(Analyzer)

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ")
set(OpenCV_DIR "D:\\BaiduNetdiskDownload\\download\\VideoAnalyzer_v3\\BXC_VideoAnalyzer_v3.2\\Analyzer\\3rdparty\\opencv\\x64\\vc16\\lib") 

set(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


set(INCLUDE_DIR /usr/local/include)
set(LIB_DIR /usr/local/lib)

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})


find_package(OpenCV REQUIRED)

set(source
        Core/Alarm.cpp
        Core/Algorithm.cpp
        Core/AlgorithmOpenVINO.cpp
        Core/AlgorithmTensorRT.cpp
        Core/Analyzer.cpp
        Core/AvPullStream.cpp
        Core/AvPushStream.cpp
        Core/Config.cpp
        Core/Frame.cpp
        Core/Scheduler.cpp
        Core/Server.cpp
        Core/Worker.cpp
        Core/Utils/CalcuIOU.cpp
        Core/Utils/Request.cpp
        Core/Utils/TensorRTEngine.cpp
        main.cpp
        )

add_executable(Analyzer ${source})
target_link_libraries(Analyzer ${OpenCV_LIBS})
target_link_libraries(Analyzer event curl jsoncpp avformat avcodec avutil swscale swresample openvino )






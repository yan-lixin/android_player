cmake_minimum_required(VERSION 3.4.1)

file(GLOB SOURCE *.cpp)

add_library(
        native-lib
        SHARED
        ${SOURCE})

find_library(
        log-lib
        log)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}.")

target_link_libraries(
        native-lib
        avfilter avformat avcodec avutil swresample swscale
        ${log-lib}
        android
        log
        z
        OpenSLES
        )
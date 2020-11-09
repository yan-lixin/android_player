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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")
#set(my_lib_path  ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${my_lib_path}")

target_link_libraries(
        native-lib
        avfilter avformat avcodec avutil swresample swscale
        ${log-lib}
        android
        log
        z
        OpenSLES
        )
cmake_minimum_required(VERSION 3.23)
project(middleware)

add_executable(${PROJECT_NAME} examples/middleware.cpp)
add_library(V4L2DeviceLib SHARED src/video/v4l2_device.cpp)
add_library(V4L2UtilsLib SHARED src/video/v4l2_utils.cpp)

target_link_libraries(${PROJECT_NAME} V4L2DeviceLib V4L2UtilsLib)

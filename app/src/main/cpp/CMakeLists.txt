cmake_minimum_required(VERSION 3.22.1)
project(kakaolink_capture LANGUAGES CXX)

add_library(capture SHARED capture.cpp)

target_compile_features(capture PRIVATE cxx_std_17)
target_link_libraries(capture PRIVATE android)

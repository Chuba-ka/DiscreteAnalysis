cmake_minimum_required(VERSION 3.16)
project(BwtMtfRle LANGUAGES CXX)

add_executable(bwt_mtf_rle main.cpp)
target_compile_features(bwt_mtf_rle PRIVATE cxx_std_17)
set_target_properties(bwt_mtf_rle PROPERTIES CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bwt_mtf_rle PRIVATE -Wall -Wextra -Wpedantic)
endif()

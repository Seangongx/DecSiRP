cmake_minimum_required(VERSION 3.16)

project(vizAccConv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(DGtal REQUIRED)

add_executable(viz
    src/vizAccConv.cpp
)

target_include_directories(viz PRIVATE
    ext
)

target_link_libraries(viz PRIVATE
    DGtal::DGtal
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/random.png
    ${CMAKE_CURRENT_BINARY_DIR}/random.png
    COPYONLY
)


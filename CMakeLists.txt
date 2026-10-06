cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(DGtal REQUIRED)


add_executable(viz vizAccConv.cpp)
target_link_libraries(viz
        DGtal::DGtal
)



cmake_minimum_required(VERSION 3.16)

project(vizAccConv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(DGtal REQUIRED)


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ih"
    "${CMAKE_CURRENT_SOURCE_DIR}/managers/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/managers/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/managers/*.ih"
)

set(IH_FILES
    ${SRC_FILES}
)

list(FILTER IH_FILES INCLUDE REGEX "\\.ih$")

set_source_files_properties(${IH_FILES} PROPERTIES
    LANGUAGE CXX
    HEADER_FILE_ONLY ON
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
             FILES ${SRC_FILES})


add_executable(decSiRP
    src/decSiRP.cpp
    ${SRC_FILES}
)


target_include_directories(decSiRP PRIVATE
    ext
)

target_link_libraries(decSiRP PRIVATE
    DGtal::DGtal
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/random.png
    ${CMAKE_CURRENT_BINARY_DIR}/random.png
    COPYONLY
)


cmake_minimum_required(VERSION 3.16)

project(DecSiRP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(DGtal REQUIRED)

# Core of the method, independent of any viewer.
add_executable(decSiRP
  src/decSiRP.cpp
  src/core/InputSurface.cpp
  src/core/VotingScheme.cpp
  src/core/InteriorVoxelClustering.cpp
  src/core/LabelExtension.cpp
  src/core/Decomposition.cpp
)

# Interactive viewer, available if DGtal was built with Polyscope.
if(DGTAL_WITH_POLYSCOPE_VIEWER)
  target_sources(decSiRP PRIVATE src/viewer/Viewer.cpp)
  target_compile_definitions(decSiRP PRIVATE DECSIRP_WITH_VIEWER)
  # Colormap used by the viewer, loaded from the working directory.
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/random.png
                 ${CMAKE_CURRENT_BINARY_DIR}/random.png COPYONLY)
else()
  message(STATUS "DGtal built without Polyscope: the viewer is disabled.")
endif()

target_include_directories(decSiRP PRIVATE src ext)
target_link_libraries(decSiRP PRIVATE DGtal::DGtal)

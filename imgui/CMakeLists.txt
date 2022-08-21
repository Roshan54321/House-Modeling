file(GLOB SOURCES "*.cpp")
include_directories(${MyProject_SOURCE_DIR}/glfw/include)
add_library(imgui ${SOURCES})
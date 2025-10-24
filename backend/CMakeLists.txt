cmake_minimum_required(VERSION 3.15)
project(hw_analyzer_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find required packages
find_package(Threads REQUIRED)

# Add executable
add_executable(hw_analyzer_backend
    src/main.cpp
    src/serial_interface.cpp
)

# Include directories
target_include_directories(hw_analyzer_backend PRIVATE 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Link libraries
target_link_libraries(hw_analyzer_backend PRIVATE
    Threads::Threads
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(hw_analyzer_backend PRIVATE ws2_32)
endif()

# Installation
install(TARGETS hw_analyzer_backend DESTINATION bin)

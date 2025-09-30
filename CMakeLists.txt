cmake_minimum_required(VERSION 3.15)
project(KineticSculpture)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ใช้ vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" 
    CACHE STRING "Vcpkg toolchain file")

# หา packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Source files
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/shader.h
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    glad::glad
    glm::glm
)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders 
     DESTINATION ${CMAKE_BINARY_DIR})
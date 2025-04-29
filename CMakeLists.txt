cmake_minimum_required(VERSION 3.15)
project(MyCrowProject)

# Atur versi Windows minimal (untuk menghilangkan warning _WIN32_WINNT)
add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7

# Standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Untuk menghilangkan warning Unicode
if(MSVC)
    add_compile_options(/utf-8)  # Gunakan UTF-8 encoding
endif()

# Include manual
include_directories(
    ${CMAKE_SOURCE_DIR}/crow/include
    ${CMAKE_SOURCE_DIR}/include
)

# Executable
add_executable(MyCrowProject src/main.cpp)

# Link library
target_link_libraries(MyCrowProject PRIVATE ws2_32 mswsock)

# Untuk Visual Studio (jika digunakan)
if(MSVC)
    target_compile_options(MyCrowProject PRIVATE /wd4566)  # Disable warning C4566
endif()
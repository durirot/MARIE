cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

project(template)
add_executable(${PROJECT_NAME} main.cpp)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#10.0.0")

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    fmt::fmt
)
target_include_directories(${PROJECT_NAME} 
    PRIVATE ${fmt_SOURCE_DIRS}/include
)


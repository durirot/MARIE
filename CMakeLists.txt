cmake_minimum_required(VERSION 3.22.0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

project(marievm)
add_executable(${PROJECT_NAME} src/main.cpp src/marie.cpp src/assemble.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
	COMPILE_OPTIONS -fsanitize=address
	LINK_OPTIONS -fsanitize=address
)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#10.0.0")

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    fmt::fmt
)
target_include_directories(${PROJECT_NAME} 
    PRIVATE ${fmt_SOURCE_DIRS}/include
)


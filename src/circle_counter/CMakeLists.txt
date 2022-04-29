cmake_minimum_required(VERSION 3.12.0)
project(Circle_Counter VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable("${PROJECT_NAME}" src/main.cpp)
target_include_directories("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include")

set_target_properties("${PROJECT_NAME}"
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wshadow)

find_package(TBB REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC "${TBB_LIB}")

find_package(fmt)
target_link_libraries("${PROJECT_NAME}" PUBLIC fmt::fmt)

find_package(OpenCV REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC "${OpenCV_LIBS}")

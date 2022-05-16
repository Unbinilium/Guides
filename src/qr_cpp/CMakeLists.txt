cmake_minimum_required(VERSION 3.12.0)
project(qr_cpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wshadow)
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

include(FetchContent)

find_package(TBB REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${TBB_LIB})

find_package(Torch REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${TORCH_LIBRARIES})

find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})

find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)

find_package(Threads REQUIRED)

FetchContent_Declare(asio GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git)
FetchContent_GetProperties(asio)
if (NOT asio_POPULATED)
    FetchContent_Populate(asio)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
    target_link_libraries(asio INTERFACE Threads::Threads)
endif ()
target_link_libraries(${PROJECT_NAME} PUBLIC asio)

find_package(OpenGL REQUIRED)

find_package(glfw3 REQUIRED)

FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git)
FetchContent_MakeAvailable(glad)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR})
    file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
    file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
    add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_SOURCES} ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
    target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR})
    target_link_libraries(imgui ${OPENGL_LIBRARIES} glfw glad)
endif ()
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

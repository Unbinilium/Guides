#include <stdio.h>

#include <queue>
#include <thread>

#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <opencv2/opencv.hpp>

#include "qr_cpp.hpp"

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "QR CPP", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    bool err = gladLoadGL() == 0;

    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_wechat_qr_window = false;
    bool show_opencv_qr_window = false;

    cv::VideoCapture cap;
    cv::Mat image, wechat_qr_result, opencv_qr_result;

    std::queue<std::thread> threads_pool;

    auto opencv_qr_app = qr_cpp::CV_App();
    auto wechat_qr_app = qr_cpp::Wechat_App();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("QR CPP");

            ImGui::Text("Set video file path or camera index:");
            static char buf[128] = "0";
            ImGui::InputText("path/id", buf, IM_ARRAYSIZE(buf));

            if (cap.isOpened()) {
                cap.read(image);
                if (image.empty()) cap.release();
            }

            if (ImGui::Button("LOAD")) {
                if (!cap.isOpened()) {
                    try { cap.open(buf); } catch (const std::exception& e) {
                        printf("Error: %s\n", e.what());
                        cap.release();
                    }

                    if (cap.isOpened()) {
                        while (!threads_pool.empty()) {
                            threads_pool.front().detach();
                            threads_pool.pop();
                        }
                        if (show_wechat_qr_window) threads_pool.push(std::thread([&]() {
                            while (cap.isOpened()) {
                                if (image.empty()) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(15));
                                    continue;
                                }
                                wechat_qr_app.detect(image);
                                wechat_qr_app.visualize();
                                wechat_qr_result = wechat_qr_app.get_image();
                            }
                        }));
                        if (show_opencv_qr_window) threads_pool.push(std::thread([&]() {
                            while (cap.isOpened()) {
                                if (image.empty()) {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(15));
                                    continue;
                                }
                                opencv_qr_app.detect(image);
                                opencv_qr_app.visualize();
                                opencv_qr_result = opencv_qr_app.get_image();
                            }
                        }));
                    }
                }
            }

            ImGui::Spacing();
            ImGui::Text("Choose which window to show:");
            ImGui::Checkbox("WeChat QR Code Detector Window", &show_wechat_qr_window);
            ImGui::Checkbox("OpenCV QR Code Detector Window", &show_opencv_qr_window);

            ImGui::Spacing();
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (cap.isOpened() && show_wechat_qr_window && !wechat_qr_result.empty()) {
            ImGui::Begin("WeChat QR Code Detector Window");
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, wechat_qr_result.cols, wechat_qr_result.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, wechat_qr_result.data);
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(wechat_qr_result.cols, wechat_qr_result.rows));
            ImGui::End();

            auto times = wechat_qr_app.get_times();
            if (!times.empty()) {
                auto result = wechat_qr_app.get_results();
                auto i = 0;
                auto times_vec = std::vector<float>(times.begin(), times.end());
                float sum = 0.0f;
                for (const auto t : times_vec) sum += t;
                char overlay[32];
                sprintf(overlay, "avg %fms", sum / times_vec.size());
                ImGui::Begin("WeChat QR Code Detection Time");
                ImGui::PlotLines("Times plot", times_vec.data(), times_vec.size(), 0, overlay, 0.0f, 500.0f, ImVec2(0, 300.0f));
                ImGui::Spacing();
                for (const auto& s : result) ImGui::Text("Result %i: %s", i++, s.c_str());
                ImGui::End();
            }
        }

        if (cap.isOpened() && show_opencv_qr_window && !opencv_qr_result.empty()) {
            ImGui::Begin("OpenCV QR Code Detector Window");
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opencv_qr_result.cols, opencv_qr_result.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, opencv_qr_result.data);
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture)), ImVec2(opencv_qr_result.cols, opencv_qr_result.rows));
            ImGui::End();

            auto times = opencv_qr_app.get_times();
            if (!times.empty()) {
                auto result = opencv_qr_app.get_results();
                auto i = 0;
                auto times_vec = std::vector<float>(times.begin(), times.end());
                float sum = 0.0f;
                for (const auto t : times_vec) sum += t;
                char overlay[32];
                sprintf(overlay, "avg %fms", sum / times_vec.size());
                ImGui::Begin("OpenCV QR Code Detection Time");
                ImGui::PlotLines("Times plot", times_vec.data(), times_vec.size(), 0, overlay, 0.0f, 500.0f, ImVec2(0, 300.0f));
                ImGui::Spacing();
                for (const auto& s : result) ImGui::Text("Result %i: %s", i++, s.c_str());
                ImGui::End();
            }
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    cap.release();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

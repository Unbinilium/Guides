#pragma once

#include <vector>
#include <string>
#include <utility>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>

namespace opi {
    inline static void cv_version(void) {
        std::cout << "[OPI] opencv version: " << cv::getVersionString() << std::endl;
    }
    
    class model {
    public:
        inline model(const char* model_path, const cv::Size kernel_size, const char* input_layer_name, const char* output_layer_name) {
            model::load(model_path);
            model::layers();
            
            this->kernel_size       = kernel_size;
            this->input_layer_name  = input_layer_name;
            this->output_layer_name = output_layer_name;
        }
        
        inline cv::Mat&& inferring(const cv::Mat& input) {
            cv::resize(input, tmp, kernel_size);
            cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
            cv::threshold(tmp, tmp, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            
            opencv_net.setInput(cv::dnn::blobFromImage(tmp), input_layer_name);
            
            return std::move(opencv_net.forward(output_layer_name));
        }
        
    protected:
        inline void load(const char* model_path) {
            this->model_path = model_path;
            opencv_net = cv::dnn::readNetFromTensorflow(this->model_path);
            
            if (!opencv_net.empty()) {
                std::cout << "[OPI] load model success: " << this->model_path << std::endl;
            } else {
                std::cout << "[OPI] load model failed: " << this->model_path << std::endl;
                return;
            }
            
#if __has_include(<opencv2/gpu/gpu.hpp>)
            opencv_net.setPreferableBackend(cv::dnn::DNN_TARGET_CUDA);
#else
            opencv_net.setPreferableBackend(cv::dnn::DNN_TARGET_OPENCL);
#endif
        }
        
        inline void layers(void) {
            if (opencv_net.empty()) {
                std::cout << "[OPI] model is empty" << std::endl;
                return;
            }
            
            std::cout << "[OPI] model from path: " << this->model_path << std::endl;
            std::cout << "[OPI] model layers: " << std::endl;
            for (const auto& layer_name : opencv_net.getLayerNames()) {
                std::cout << "\t\t" << layer_name << std::endl;
            }
        }
        
    private:
        cv::dnn::Net opencv_net;
        const char*  model_path;
        
        const char*  input_layer_name;
        const char*  output_layer_name;
        
        cv::Size     kernel_size;
        cv::Mat      tmp;
    };
}

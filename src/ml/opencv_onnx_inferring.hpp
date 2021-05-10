#pragma once

#include <utility>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>

namespace ooi {
    inline static void cv_version(void) {
        std::cout << "[OOI] opencv version: " << cv::getVersionString() << std::endl;
    }
    
    class model {
    public:
        inline model(const char* onnx_model_path, const cv::Size& input_size) {
            model::load(onnx_model_path);
            model::layers();
            
            this->input_size = input_size;
        }
        
        inline int inferring(const cv::Mat& input) {
            cv::resize(input, tmp, input_size);
            cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
            cv::threshold(tmp, tmp, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            
            opencv_net.setInput(cv::dnn::blobFromImage(tmp));
            
            float max_probability     { 0 };
            int   max_probability_idx { 0 };
            int   i                   { -1 };
            opencv_net.forward().forEach<float>([&max_probability, &max_probability_idx, &i](float &data, [[maybe_unused]] const int * position) -> void {
                if (++i) {
                    if (data > max_probability) {
                        max_probability     = data;
                        max_probability_idx = i;
                    }
                } else {
                    max_probability = data;
                }
            });
            
            return max_probability_idx;
        }
        
    protected:
        inline void load(const char* onnx_model_path) {
            model_path = onnx_model_path;
            opencv_net = cv::dnn::readNetFromONNX(model_path);
            
            if (!opencv_net.empty()) {
                std::cout << "[OOI] load model success: " << model_path << std::endl;
            } else {
                std::cout << "[OOI] load model failed: " << model_path << std::endl;
                return;
            }
            
#if __has_include(<opencv2/gpu/gpu.hpp>)
            opencv_net.setPreferableBackend(cv::dnn::DNN_TARGET_CUDA);
#else
            opencv_net.setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
#endif
        }
        
        inline void layers(void) {
            if (opencv_net.empty()) {
                std::cout << "[OOI] model is empty" << std::endl;
                return;
            }
            
            std::cout << "[OOI] model from " << model_path << " has layers: " << std::endl;
            for (const auto& layer_name : opencv_net.getLayerNames()) {
                std::cout << "\t\t" << layer_name << std::endl;
            }
        }
        
    private:
        cv::dnn::Net opencv_net;
        const char*  model_path;
        
        cv::Size     input_size;
        cv::Mat      tmp;
    };
}

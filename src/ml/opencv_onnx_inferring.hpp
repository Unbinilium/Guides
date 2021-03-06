/*
 * @name: opencv_onnx_inferring.hpp
 * @namespace: ooi
 * @class: model
 * @brief: Load ONNX model and infer input image for classified int digit
 * @author Unbinilium
 * @version 1.0.3
 * @date 2021-05-10
 */

#pragma once

// #define OOI_SHOW_INFERRING_BINARY
// #define OOI_SHOW_INFERRING_RESULTS_RAW

#include <iostream>

#include <opencv2/opencv.hpp>

namespace ooi {
    class model {
    public:
        /*
         @brief: Init model from params
         @param: onnx_model_path, the path of the modle on your machine, downloadable at https://github.com/onnx/models/blob/master/vision/classification/mnist/model/mnist-8.onnx
         @param: input_size, define the input layer size, default to cv::Size(28, 28)
         */
        inline model(const char* onnx_model_path, const cv::Size& input_size = cv::Size(28, 28)) {
            model::load(onnx_model_path);
            model::layers();
            
            this->input_size = input_size;
        }
        
        /*
         @brief:  Inferring input image from loaded model, return classified int digit
         @param:  input, the image to classify (only 1 digit), const reference from cv::Mat
         @param:  median_blur_kernel_size, define the kernel size of median blur pre-processing, default to int 3, set 0 to disable
         @param:  probability_threshold, the min probability of considerable probability to iterate, determined by the model, mnist-8.onnx has the output array from -1e4 to 1e4, default is 0
         @return: max_probability_idx, the most probable digit classified from input image in int type, -1 means all the probability is out of the threahold
         */
        inline int inferring(const cv::Mat& input, const int median_blur_kernel_size = 3, const float probability_threshold = 0) {
            cv::resize(input, tmp, input_size);
            if (tmp.channels() != 1) {
                cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
            }
            if (median_blur_kernel_size != 0) {
                cv::medianBlur(tmp, tmp, median_blur_kernel_size);
            }
            cv::threshold(tmp, tmp, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            
#ifdef OOI_SHOW_INFERRING_BINARY
            cv::imshow("[OOI] inferring_binary", tmp);
#endif
            opencv_net.setInput(cv::dnn::blobFromImage(tmp));
            
            float max_probability     { 0 };
            int   max_probability_idx { 0 };
            int   i                   { -1 };
            opencv_net.forward().forEach<float>([&probability_threshold, &max_probability, &max_probability_idx, &i](float &data, [[maybe_unused]] const int * position) -> void {
                    if (++i) {
                        if (data > max_probability) {
                            max_probability     = data;
                            max_probability_idx = i;
                        }
                    } else {
                        if (data > probability_threshold) {
                            max_probability = data;
                        } else {
                            max_probability = probability_threshold;
                            max_probability_idx = -1;
                        }
                    }
                
#ifdef OOI_SHOW_INFERRING_RESULTS_RAW
                std::cout << "[OOI] inffering results: " << i << " -> " << data << std::endl;
#endif
            });
            
            return max_probability_idx;
        }
        
    protected:
        /*
         @brief: Load model from onnx_model_path
         @param: onnx_model_path, the path of the modle on your machine, downloadable at https://github.com/onnx/models/blob/master/vision/classification/mnist/model/mnist-8.onnx
         */
        inline void load(const char* onnx_model_path) {
            std::cout << "[OOI] opencv version: " << cv::getVersionString() << std::endl;
            
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
        /*
         @brief: Print model layers detail from loaded model
         */
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

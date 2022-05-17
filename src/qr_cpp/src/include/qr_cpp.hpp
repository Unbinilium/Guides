#pragma once

#include <mutex>
#include <shared_mutex>
#include <deque>
#include <string>
#include <chrono>
#include <utility>
#include <iostream>

#include <opencv2/opencv.hpp>

namespace qr_cpp {
    template <typename T>
    class ApplicationBase {
    public:
        ApplicationBase() = default;
        ~ApplicationBase() = default;

        void visualize() {
            if (points_.empty()) return;
            std::unique_lock lock(mutex_);
            for (std::size_t i = 0; i < points_.size(); i += 4) {
                std::vector<cv::Point2i> contour;
                for (int j = 0; j != 4; ++j) contour.push_back(cv::Point2i(points_[i + j].x, points_[i + j].y));
                cv::polylines(image_, {contour}, true, cv::Scalar(0, 255, 0), 2, cv::LINE_8, 0);
                for (const auto& point : contour) cv::circle(image_, point, 3, cv::Scalar(0, 0, 255), -1);
            }
        }

        auto get_results() {
            std::shared_lock lock(mutex_);
            return results_;
        }

        auto get_image() {
            cv::Mat image;
            std::shared_lock lock(mutex_);
            cv::cvtColor(image_, image, cv::COLOR_BGR2RGBA);
            return image;
        }

        auto get_times() {
            std::shared_lock lock(mutex_);
            return times_;
        }

    protected:
        void time_start() { start_ = std::chrono::high_resolution_clock::now(); }

        void time_end() {
            end_ = std::chrono::high_resolution_clock::now();
            std::unique_lock lock(mutex_);
            while (times_.size() >= 15) times_.pop_front();
            times_.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count());
        }

        T* detector_;
        cv::Mat image_;
        std::vector<cv::Point2f> points_;
        std::vector<std::string> results_;
        std::vector<cv::Mat> straight_images_;
        std::chrono::high_resolution_clock::time_point start_;
        std::chrono::high_resolution_clock::time_point end_;
        std::deque<float> times_;
        std::shared_mutex mutex_;
    };

    class CV_App : public ApplicationBase<cv::QRCodeDetector> {
    public:
        CV_App() { detector_ = new cv::QRCodeDetector; }
        ~CV_App() { delete detector_; }

        void detect(const cv::Mat& image) {
            {
                std::unique_lock lock(mutex_);
                image_ = image.clone();
            }
            auto results = std::vector<std::string>();
            time_start();
            detector_->detectAndDecodeMulti(image_, results, points_, straight_images_);
            time_end();
            std::unique_lock lock(mutex_);
            results_ = results;
        }
    };

    class WC_App : public ApplicationBase<cv::wechat_qrcode::WeChatQRCode> {
    public:
        WC_App() { detector_ = new cv::wechat_qrcode::WeChatQRCode; }
        ~WC_App() { delete detector_; }

        void detect(const cv::Mat& image) {
            {
                std::unique_lock lock(mutex_);
                image_ = image.clone();
            }
            auto results = std::vector<std::string>();
            time_start();
            results = detector_->detectAndDecode(image_, points_);
            time_end();
            std::unique_lock lock(mutex_);
            results_ = results;
        }
    };
}

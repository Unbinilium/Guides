#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <variant>
#include <valarray>
#include <execution>
#include <algorithm>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/fast_math.hpp>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "config_handler.hpp"
#include "console_base.hpp"
#include "editor_base.hpp"

namespace cc {
    class CircleCounter : public ConsoleBase, public EditorBase {
    public:
        CircleCounter(
            const std::string path, cc::ConfigHandler* config_handler
        ) : _path(path), _p_config_handler(config_handler), _is_cap(path.length() == 1), _is_updated(false) {
            _trackbar_container    = types::bridge::trackbar{
                ._p_config_handler = _p_config_handler,
                ._p_is_updated     = &_is_updated
            };
            _mouse_container       = types::bridge::mouse{
                ._p_edited_points  = &_edited_points,
                ._p_is_updated     = &_is_updated
            };
        }

        ~CircleCounter() {
            if (_is_cap) _cap.release();
        };

        void load() {
            if (_is_cap) {
                _cap   = cv::VideoCapture(std::stoi(_path));
                _cap.read(_img);
                _frame = _img.clone();
            } else {
                _img   = cv::imread(_path.c_str(), cv::IMREAD_COLOR);
                _frame = _img.clone();
            }
        }

        auto is_updated() {
            return _is_cap ? !_cap.read(_img) : _is_updated.load();
        }

        void process() {
            _p_config_handler->get_mutex().lock();
            const auto config{_p_config_handler->link()};
            _p_config_handler->get_mutex().unlock();
            cv::cvtColor(_img, _gray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(_gray, _blur,
                cv::Size(std::get<int>(config.at("gaussian_kernel_w").at("value")), std::get<int>(config.at("gaussian_kernel_h").at("value"))),
                std::get<double>(config.at("gaussian_sigma_x").at("value")),
                std::get<double>(config.at("gaussian_sigma_y").at("value"))
            );
            _circles.clear();
            cv::HoughCircles(_blur, _circles,
                cv::HOUGH_GRADIENT,
                std::get<double>(config.at("hough_dp").at("value")),
                std::get<double>(config.at("hough_min_dist").at("value")),
                std::get<double>(config.at("hough_p1").at("value")),
                std::get<double>(config.at("hough_p2").at("value")),
                std::get<int>   (config.at("hough_min_radis").at("value")),
                std::get<int>   (config.at("hough_max_radis").at("value"))
            );
            _is_updated.store(false);
        }

        void visualize() {
            _frame = _img.clone();
            const auto avg_radius{(std::get<int>(
                _p_config_handler->link().at("hough_min_radis").at("value")) + std::get<int>(_p_config_handler->link().at("hough_max_radis").at("value"))
            ) / 2};
            for (const auto& point : _edited_points) {
                auto it{std::remove_if(std::execution::par, _circles.begin(), _circles.end(),
                    [&](const auto& circle) {
                        return std::pow(point.x - circle[0], 2) + std::pow(point.y - circle[1], 2) <= std::pow(circle[2], 2);
                    }
                )};
                if (it != _circles.end()) _circles.erase(it, _circles.end());
                else                      _circles.push_back(cv::Vec3i(point.x, point.y, avg_radius));
            }
            {
                using namespace std::string_literals;
                cv::putText(_frame,
                    "Circle Count: "s + std::to_string(_circles.size()),
                    cv::Point(30, 80),
                    cv::FONT_HERSHEY_SIMPLEX,
                    2.0,
                    cv::Scalar(255, 255, 255),
                    3
                );
            }
            std::for_each(std::execution::par_unseq, _circles.begin(), _circles.end(),
                [&](const auto& c) {
                    const auto center{cv::Point2i(::cvRound(c[0]), ::cvRound(c[1]))};
                    const auto radius{::cvRound(c[2])};
                    cv::circle(_frame, center, 2,      cv::Scalar(0, 0, 255), -1);
                    cv::circle(_frame, center, radius, cv::Scalar(0, 255, 0),  1);
                }
            );
            _is_updated.store(false);
        }

        void display() {
            cv::namedWindow("Circle Counter", cv::WINDOW_NORMAL);
            cv::resizeWindow("Circle Counter", _frame.size());
            cv::imshow("Circle Counter", _frame);
        }

        bool is_display_open() {
            return static_cast<int>(cv::getWindowProperty("Circle Counter", cv::WND_PROP_VISIBLE)) != -1;
        }

        void update() {
            cv::imshow("Circle Counter", _frame);
            _is_updated.store(true);
        }

        void snapshot() { cv::imwrite(_path + ".snapshot.png", _frame); }

    private:
        const std::string        _path;
        cc::ConfigHandler*       _p_config_handler;
        cv::VideoCapture         _cap;
        bool                     _is_cap;
        cv::Mat                  _img;
        cv::Mat                  _gray;
        cv::Mat                  _blur;
        std::vector<cv::Vec3f>   _circles;
        std::vector<cv::Point2i> _edited_points;
        cv::Mat                  _frame;
        std::atomic_bool         _is_updated;
    };
}

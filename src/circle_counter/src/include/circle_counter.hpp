#pragma once

#include <vector>
#include <atomic>
#include <string>
#include <variant>
#include <valarray>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/fast_math.hpp>

#include "config_handler.hpp"
#include "console_base.hpp"
#include "editor_base.hpp"

namespace cc {
    class CircleCounter : public ConsoleBase, public EditorBase {
    public:
        CircleCounter(
            const std::string img_path, cc::ConfigHandler* config_handler
        ) : _img_path(img_path), _p_config_handler(config_handler), _is_updated(false) {
            __p_img            = &_img;
            __p_config_handler = _p_config_handler;
            __p_is_updated     = &_is_updated;

            _mouse_container   = types::callback_container::mouse{
                ._p_edited_points = &_edited_points,
                ._p_is_updated    = &_is_updated
            };
        }

        ~CircleCounter() = default;

        void load() {
            _img   = cv::imread(_img_path.c_str(), cv::IMREAD_COLOR);
            _frame = _img.clone();
        }

        auto is_updated() { return _is_updated.load(); }

        void process() {
            _p_config_handler->get_mutex().lock();
            const auto config{_p_config_handler->link()};
            _p_config_handler->get_mutex().unlock();
            cv::cvtColor(_img, _gray, cv::COLOR_BGR2GRAY);
            cv::GaussianBlur(
                _gray,
                _blur,
                cv::Size(
                    std::get<int>(config.at("gaussian_kernel_w_o")),
                    std::get<int>(config.at("gaussian_kernel_h_o"))
                ),
                std::get<double>(config.at("gaussian_sigma_x_d")),
                std::get<double>(config.at("gaussian_sigma_y_d"))
            );
            _circles.clear();
            cv::HoughCircles(
                _blur,
                _circles,
                cv::HOUGH_GRADIENT,
                std::get<double>(config.at("hough_dp_d")),
                std::get<double>(config.at("hough_min_dist_d")),
                std::get<double>(config.at("hough_p1_d")),
                std::get<double>(config.at("hough_p2_d")),
                std::get<int>(config.at("hough_min_radis")),
                std::get<int>(config.at("hough_max_radis"))
            );
            _is_updated.store(false);
        }

        void visualize() {
            _frame = _img.clone();
            const auto avg_radius{(std::get<int>(_p_config_handler->link().at("hough_min_radis")) + std::get<int>(_p_config_handler->link().at("hough_max_radis"))) / 2};
            for (const auto& point : _edited_points) {
                auto it{std::remove_if(
                    _circles.begin(), _circles.end(),
                    [&](const auto& circle) {
                        return std::pow(point.x - circle[0], 2) + std::pow(point.y - circle[1], 2) <= std::pow(circle[2], 2);
                    }
                )};
                if (it != _circles.end()) _circles.erase(it, _circles.end());
                else _circles.push_back(cv::Vec3i(point.x, point.y, avg_radius));
            }
            for (const auto& c : _circles) {
                const auto center{cv::Point2i(::cvRound(c[0]), ::cvRound(c[1]))};
                const auto radius{::cvRound(c[2])};

                cv::circle(_frame, center, 2,      cv::Scalar(0, 0, 255), -1);
                cv::circle(_frame, center, radius, cv::Scalar(0, 255, 0),  1);
            }
            using namespace std::string_literals;
            cv::putText(
                _frame,
                "Circle Count: "s + std::to_string(_circles.size()),
                cv::Point(30, 80),
                cv::FONT_HERSHEY_SIMPLEX,
                2.0,
                cv::Scalar(255, 255, 255),
                3.0
            );
            _is_updated.store(false);
        }

        void display() {
            cv::namedWindow("Circle Counter", cv::WINDOW_AUTOSIZE);
            cv::imshow("Circle Counter", _frame);
        }

        void update() {
            cv::imshow("Circle Counter", _frame);
            _is_updated.store(true);
        }

        void restart() {
            _circles.clear();
            _edited_points.clear();
            _is_updated.store(false);
        }

    private:
        const std::string        _img_path;
        cc::ConfigHandler*       _p_config_handler;
        cv::Mat                  _img;
        cv::Mat                  _gray;
        cv::Mat                  _blur;
        std::vector<cv::Vec3f>   _circles;
        std::vector<cv::Point2i> _edited_points;
        cv::Mat                  _frame;
        std::atomic_bool         _is_updated;
    };
}

#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <variant>
#include <algorithm>
#include <unordered_map>

#include <opencv2/highgui.hpp>

#include "config_handler.hpp"

namespace cc {
    namespace types {
        constexpr uint8_t type_int            = 0;
        constexpr uint8_t type_float          = 1;
        constexpr uint8_t type_double         = 2;
        constexpr uint8_t type_odd            = 3;
        constexpr uint8_t float_2_int_factor  = 10;
        constexpr uint8_t double_2_int_factor = 10;

        namespace callback_container {
            struct trackbar {
                std::string       _config_key;
                uint8_t           _config_type;
                int               _pos;
                ConfigHandler*    _p_config_handler;
                std::atomic_bool* _p_is_updated;
            };
        }
    }

    class ConsoleBase {
    public:
        void console() {
            cv::namedWindow("Console", cv::WINDOW_GUI_EXPANDED);
            for (const auto& v : __p_config_handler->link())
                _config_adapter.insert({
                    v.first,
                    types::callback_container::trackbar{
                        ._config_key       = v.first,
                        ._pos              = [&] {
                            int value{};
                            std::visit([&](auto&& arg) {
                                using T = std::decay_t<decltype(arg)>;
                                if constexpr (std::is_same_v<T, int>)         value = static_cast<int>(arg);
                                else if constexpr (std::is_same_v<T, float>)  value = static_cast<int>(arg * types::float_2_int_factor);
                                else if constexpr (std::is_same_v<T, double>) value = static_cast<int>(arg * types::double_2_int_factor);
                            }, v.second);
                            return value;
                        }(),
                        ._p_config_handler = __p_config_handler,
                        ._p_is_updated     = __p_is_updated
                    }
                });

            {
                auto& ref_config{_config_adapter["gaussian_kernel_w_o"]};
                ref_config._config_type = types::type_odd;
                cv::createTrackbar("Bulr Kernel Width", "Console", &ref_config._pos, 100, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["gaussian_kernel_h_o"]};
                ref_config._config_type = types::type_odd;
                cv::createTrackbar("Bulr Kernel Height", "Console", &ref_config._pos, 100, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["gaussian_sigma_x_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Bulr Sigma X", "Console", &ref_config._pos, 200, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["gaussian_sigma_y_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Bulr Sigma Y", "Console", &ref_config._pos, 200, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_dp_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Hough Circle DP", "Console", &ref_config._pos, 100, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_min_dist_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Minimal Circle Distance", "Console", &ref_config._pos, 1000, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_p1_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Hough Circle Parm1", "Console", &ref_config._pos, 5000, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_p2_d"]};
                ref_config._config_type = types::type_double;
                cv::createTrackbar("Hough Circle Parm2", "Console", &ref_config._pos, 500, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_min_radis"]};
                ref_config._config_type = types::type_int;
                cv::createTrackbar("Minimal Circle Radius", "Console", &ref_config._pos, 100, trackbar_callback, &ref_config);
            } {
                auto& ref_config{_config_adapter["hough_max_radis"]};
                ref_config._config_type = types::type_int;
                cv::createTrackbar("Maximal Circle Radius", "Console", &ref_config._pos, 100, trackbar_callback, &ref_config);
            }
        }

    protected:
        cv::Mat*           __p_img;
        cc::ConfigHandler* __p_config_handler;
        std::atomic_bool*  __p_is_updated;

    private:
        std::unordered_map<std::string, types::callback_container::trackbar> _config_adapter;

        static void trackbar_callback(const int pos, void* container) {
            auto trackbar{static_cast<types::callback_container::trackbar*>(container)};
            auto lock{std::lock_guard(trackbar->_p_config_handler->get_mutex())};
            switch (trackbar->_config_type) {
            case types::type_int:
                trackbar->_p_config_handler->link()[trackbar->_config_key] = static_cast<int>(trackbar->_pos);
                break;
            case types::type_float:
                trackbar->_p_config_handler->link()[trackbar->_config_key] = static_cast<float>(trackbar->_pos / static_cast<float>(types::float_2_int_factor));
                break;
            case types::type_double:
                trackbar->_p_config_handler->link()[trackbar->_config_key] = static_cast<double>(trackbar->_pos / static_cast<double>(types::double_2_int_factor));
                break;
            case types::type_odd:
                trackbar->_p_config_handler->link()[trackbar->_config_key] = static_cast<int>(
                    trackbar->_pos & 1 ? trackbar->_pos : trackbar->_pos + 1
                );
                break;
            }
            trackbar->_p_is_updated->store(false);
        }
    };
}

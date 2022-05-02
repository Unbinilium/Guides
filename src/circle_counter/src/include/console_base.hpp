#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <variant>
#include <algorithm>

#include <opencv2/highgui.hpp>

#include "common_types.hpp"
#include "config_handler.hpp"

namespace cc {
    namespace types {
        namespace bridge {
            struct trackbar {
                ConfigHandler*    _p_config_handler;
                std::atomic_bool* _p_is_updated;
            };
        }

        namespace callback_container {
            struct trackbar {
                std::string              _config_key;
                int                      _config_type;
                int                      _pos;
                int                      _max;
                types::bridge::trackbar* _bridge;
            };
        }
    }

    class ConsoleBase {
    public:
        void console() {
            if (_config_adapter.empty()) for (const auto& v : _trackbar_container._p_config_handler->link())
                _config_adapter.push_back(
                    types::callback_container::trackbar{
                        ._config_key       = v.first,
                        ._config_type      = std::get<int>(v.second.at("type")),
                        ._pos              = [&] {
                            int value{};
                            std::visit([&](auto&& arg) {
                                using T = std::decay_t<decltype(arg)>;
                                if constexpr      (std::is_same_v<T, int>)    value = static_cast<int>(arg);
                                else if constexpr (std::is_same_v<T, float>)  value = static_cast<int>(arg * types::float_2_int_factor);
                                else if constexpr (std::is_same_v<T, double>) value = static_cast<int>(arg * types::double_2_int_factor);
                            }, v.second.at("value"));
                            return value;
                        }(),
                        ._max              = [&] {
                            int value{};
                            std::visit([&](auto&& arg) {
                                using T = std::decay_t<decltype(arg)>;
                                if constexpr      (std::is_same_v<T, int>)    value = static_cast<int>(arg);
                                else if constexpr (std::is_same_v<T, float>)  value = static_cast<int>(arg * types::float_2_int_factor);
                                else if constexpr (std::is_same_v<T, double>) value = static_cast<int>(arg * types::double_2_int_factor);
                            }, v.second.at("max"));
                            return value;
                        }(),
                        ._bridge           = &_trackbar_container
                    }
                );
            cv::namedWindow("Console", cv::WINDOW_GUI_EXPANDED);
            for (auto& v : _config_adapter) {
                const auto trackbar_name{std::get<std::string>(_trackbar_container._p_config_handler->link().at(v._config_key).at("id"))};
                cv::createTrackbar(trackbar_name, "Console", nullptr, v._max, &trackbar_callback, &v);
                cv::setTrackbarPos(trackbar_name, "Console", v._pos);
            }
        }

    protected:
        types::bridge::trackbar _trackbar_container;

    private:
        std::vector<types::callback_container::trackbar> _config_adapter;

        static void trackbar_callback(const int pos, void* container) {
            auto trackbar{static_cast<types::callback_container::trackbar*>(container)};
            if (pos != trackbar->_pos) trackbar->_pos = pos;
            else                       return;
            auto lock{std::lock_guard(trackbar->_bridge->_p_config_handler->get_mutex())};
            switch (trackbar->_config_type) {
            case types::type_int:
                trackbar->_bridge->_p_config_handler->link()[trackbar->_config_key]["value"] = static_cast<int>(pos);
                break;
            case types::type_float:
                trackbar->_bridge->_p_config_handler->link()[trackbar->_config_key]["value"] = static_cast<float>(pos / static_cast<float>(types::float_2_int_factor));
                break;
            case types::type_double:
                trackbar->_bridge->_p_config_handler->link()[trackbar->_config_key]["value"] = static_cast<double>(pos / static_cast<double>(types::double_2_int_factor));
                break;
            case types::type_odd:
                trackbar->_bridge->_p_config_handler->link()[trackbar->_config_key]["value"] = static_cast<int>(pos & 1 ? pos : pos + 1);
                break;
            }
            trackbar->_bridge->_p_is_updated->store(false);
        }
    };
}

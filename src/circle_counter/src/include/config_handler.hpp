#pragma once

#include <mutex>
#include <string>
#include <variant>
#include <unordered_map>

#include <opencv2/core/persistence.hpp>

namespace cc {
    namespace types {
        using config_object = std::variant<int, float, double>;

        std::unordered_map<std::string, cc::types::config_object> default_config{
            {"gaussian_kernel_w_o",   int()},
            {"gaussian_kernel_h_o",   int()},
            {"gaussian_sigma_x_d", double()},
            {"gaussian_sigma_y_d", double()},
            {"hough_dp_d",         double()},
            {"hough_min_dist_d",   double()},
            {"hough_p1_d",         double()},
            {"hough_p2_d",         double()},
            {"hough_min_radis",       int()},
            {"hough_max_radis",       int()}
        };
    }

    class ConfigHandler {
    public:
        ConfigHandler(
            const std::string&                                           path,
            const std::unordered_map<std::string, types::config_object>& config
        ) : _path(path), _config(config) {}

        ~ConfigHandler() { sync(); };

        void load() {
            auto fs{cv::FileStorage(_path.c_str(), cv::FileStorage::READ)};
            for (const auto& v : _config)
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    _config[v.first] = static_cast<T>(fs[v.first]);
                }, v.second);
            fs.release();
        }

        auto& link() { return _config; }

        auto& get_mutex() { return _mutex; }

        void sync() {
            auto fs{cv::FileStorage(_path.c_str(), cv::FileStorage::WRITE)};
            for (const auto& v : _config)
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    fs << v.first << std::get<T>(v.second);
                }, v.second);
            fs.release();
        }

    private:
        const std::string                                     _path;
        std::mutex                                            _mutex;
        std::unordered_map<std::string, types::config_object> _config;
    };
}

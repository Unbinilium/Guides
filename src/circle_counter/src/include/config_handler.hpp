#pragma once

#include <mutex>
#include <string>
#include <variant>
#include <stdexcept>
#include <unordered_map>

#include <opencv2/core/persistence.hpp>

#include "common_types.hpp"

namespace cc {
    namespace types {
        using config_values = std::variant<int, float, double, std::string>;
        using config_object = std::unordered_map<std::string, config_values>;
    }

    class ConfigHandler {
    public:
        ConfigHandler(const std::string& path) : _path(path) {}

        ~ConfigHandler() = default;

        void load() {
            auto fs{cv::FileStorage(_path.c_str(), cv::FileStorage::READ)};
            if (!fs.isOpened()) throw std::runtime_error("Cannot open config file: " + _path);
            for (const auto& k : fs.root().keys()) {
                _config[k]["id"]   = static_cast<std::string>(fs[k]["id"]);
                _config[k]["type"] = static_cast<int>(fs[k]["type"]);
                switch (static_cast<int>(fs[k]["type"])) {
                case types::type_int:
                case types::type_odd:
                    _config[k]["value"] = static_cast<int>(fs[k]["value"]);
                    _config[k]["max"]   = static_cast<int>(fs[k]["max"]);
                    break;
                case types::type_float:
                    _config[k]["value"] = static_cast<float>(fs[k]["value"]);
                    _config[k]["max"]   = static_cast<float>(fs[k]["max"]);
                    break;
                case types::type_double:
                    _config[k]["value"] = static_cast<double>(fs[k]["value"]);
                    _config[k]["max"]   = static_cast<double>(fs[k]["max"]);
                    break;
                }
            }
            fs.release();
        }

        auto& link() { return _config; }

        auto& get_mutex() { return _mutex; }

        void sync() {
            auto fs{cv::FileStorage(_path.c_str(), cv::FileStorage::WRITE)};
            for (const auto& v : _config)
                std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    fs << v.first << "{" <<
                        "type"  << std::get<int>        (v.second.at("type"))  <<
                        "value" << std::get<T>          (v.second.at("value")) <<
                        "max"   << std::get<T>          (v.second.at("max"))   <<
                        "id"    << std::get<std::string>(v.second.at("id"))    <<
                    "}";
                }, v.second.at("value"));
            fs.release();
        }

    private:
        const std::string                                     _path;
        std::mutex                                            _mutex;
        std::unordered_map<std::string, types::config_object> _config;
    };
}

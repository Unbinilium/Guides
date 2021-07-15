#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <iostream>

namespace ubn {
    template <typename T = std::chrono::high_resolution_clock, typename P = std::chrono::milliseconds>
    class time_utils {
    public:
        time_utils() = default;
            
        explicit time_utils(
            const std::map<std::string, std::chrono::time_point<T>> _time_point_map,
            const std::map<std::string, P> _duration_map
        ) : time_point_map_(_time_point_map), duration_map_(_duration_map) {}
            
        ~time_utils() { clear(); };
        
        inline auto operator-(const time_utils& _time_utils) {
            return getTag() - _time_utils.getTag();
        }
            
        inline auto operator+(const time_utils& _time_utils) {
            return getTag() + _time_utils.getTag();
        }
        
        inline void setTag(const std::string& _tag_name) {
            if (time_point_map_.contains(_tag_name)) {
                duration_map_.insert_or_assign(
                    _tag_name,
                    std::chrono::duration_cast<P>(
                        T::now() - time_point_map_[_tag_name]
                    )
                );
                time_point_map_[_tag_name] = T::now();
            } else {
                time_point_map_.emplace(
                    std::pair(_tag_name, T::now())
                );
            }
            updateInfo(_tag_name);
        }
        
        inline auto getTag(const std::string& _tag_name) {
            return time_point_map_.contains(_tag_name)
                ? time_point_map_[_tag_name]
                : T::now();
        }
            
        inline auto getAllTag() {
            return time_point_map_;
        }
            
        inline bool eraseTag(const std::string& _tag_name) {
            return time_point_map_.contains(_tag_name)
                ? [&]() -> bool { time_point_map_.erase(_tag_name); return true; }
                : false;
        }
        
        inline auto getDuration(const std::string& _tag_name) {
            return duration_map_.contains(_tag_name)
                ? duration_map_[_tag_name]
                : P();
        }
            
        inline auto getAllDuration() {
            return duration_map_;
        }
            
        inline bool eraseDuration(const std::string& _tag_name) {
            return duration_map_.contains(_tag_name)
                ? [&]() -> bool { 
                    duration_map_.erase(_tag_name);
                    info_history_map_.erase(_tag_name);
                    return true; }
                : false;
        }

        inline auto getInfo(const std::string& _tag_name) {
            return info_history_map_.contains(_tag_name)
                ? getInfo(_tag_name, info_history_map_[_tag_name].size() - 1)
                : std::unordered_map<std::string, double>();
        }
            
        inline void printInfo(const std::string& _tag_name) {
            if (info_history_map_.contains(_tag_name)) {
                printInfo(_tag_name, info_history_map_[_tag_name].size() - 1);
            }
        }
            
        inline void printAllInfo() {
            printAllInfo(duration_map_);
        }
            
        inline auto getInfoHistort(const std::string& _tag_name) {
            return info_history_map_.contains(_tag_name)
                ? info_history_map_[_tag_name]
                : std::vector<std::unordered_map<std::string, double>>();
        }
            
        inline void printInfoHistory(const std::string& _tag_name) {
            if (info_history_map_.contains(_tag_name)) {
                size_t i { 0 };
                for (auto& info_history : info_history_map_[_tag_name]) {
                    printInfo(_tag_name, i++);
                }
            }
        }
            
        inline void printAllInfoHistory() {
            printAllInfoHistory(duration_map_);
        }
            
        inline bool erase(const std::string& _tag_name) {
            return eraseTag(_tag_name) || eraseDuration(_tag_name);
        }
            
        inline void clear() {
            time_point_map_.clear();
            duration_map_.clear();
            info_history_map_.clear();
        }
    
    protected:
        inline void updateInfo(const std::string& _tag_name) {
            const auto duration { getDuration(_tag_name) };
            const auto duration_count { duration.count() };
            std::unordered_map<std::string, double> info;
            if (info_history_map_.contains(_tag_name)) {
                info = info_history_map_[_tag_name][info_history_map_[_tag_name].size() - 1];
                info["min_duration"] = info["avg_duration"] != 0.f && info["min_duration"] < duration_count
                    ? info["min_duration"]
                    : duration_count;
                info["max_duration"] = info["max_duration"] > duration_count
                    ? info["max_duration"]
                    : duration_count;
                info["avg_duration"] = info["avg_duration"] != 0.f
                    ? (info["avg_duration"] + duration_count) / 2.f
                    : duration_count;
            } else {
                for (const auto& info_name : { "min_duration", "max_duration", "avg_duration" }) {
                    info.emplace(std::pair(info_name, duration_count));
                }
            }
            info.insert_or_assign(
                "time_point_at",
                time_point_map_.contains(_tag_name)
                    ? time_point_map_[_tag_name].time_since_epoch().count()
                    : 0.f
            );
            info.insert_or_assign(
                "cur_duration",
                duration_count
            );
            info.insert_or_assign("frequency", 1.f / std::chrono::duration<double, std::ratio<1>>(duration).count());
            if (info_history_map_.contains(_tag_name)) {
                info_history_map_[_tag_name].emplace_back(info);
                
            } else {
                std::vector<std::unordered_map<std::string, double>> info_history { info };
                info_history_map_.emplace(std::pair(_tag_name, info_history));
            }
        }
            
        inline auto getInfo(const std::string& _tag_name, const size_t _i) {
            return info_history_map_[_tag_name][_i];
        }
        
        inline void printInfo(const std::string& _tag_name, const size_t _i) {
            auto info_history { info_history_map_[_tag_name][_i] };
            std::cout << "[time_utils] Info '"
                << _tag_name << "' -> "
                << _i << " set at: "
                << std::size_t(info_history["time_point_at"]) << " duration (cur/min/max/avg): "
                << info_history["cur_duration"] << "/"
                << info_history["min_duration"] << "/"
                << info_history["max_duration"] << "/"
                << info_history["avg_duration"] << ", frequency: "
                << info_history["frequency"] << std::endl;
        }
            
        inline void printAllInfo(const std::map<std::string, P>& _duration_map)
        {
            for (const auto& [key, _] : _duration_map) {
                printInfo(key);
            }
        }
            
        inline void printAllInfoHistory(const std::map<std::string, P>& _duration_map)
        {
            for (const auto& [key, _] : _duration_map) {
                printInfoHistory(key);
            }
        }
        
    private:
        std::map<std::string, std::chrono::time_point<T>> time_point_map_;
        std::map<std::string, P> duration_map_;
        std::map<std::string, std::vector<std::unordered_map<std::string, double>>> info_history_map_;
    };
}

#include <thread>

void doSomeThing() {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.5s);
}

int main() {
    ubn::time_utils t_u;
    
    t_u.setTag("Clock 1");
    doSomeThing();
    t_u.setTag("Clock 1");
    t_u.setTag("Clock 2");
    doSomeThing();
    doSomeThing();
    t_u.setTag("Clock 2");
    doSomeThing();
    t_u.setTag("Clock 2");
    
    t_u.printAllInfoHistory();
}

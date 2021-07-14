#include <map>
#include <chrono>
#include <thread>
#include <utility>

#include <iostream>

namespace ubn {
    template <typename T = std::chrono::high_resolution_clock, typename P = std::chrono::milliseconds>
    class time_utils {
    public:
        time_utils() = default;
            
        time_utils(
            const std::map<std::string, std::chrono::time_point<T>> _time_point_map,
            const std::map<std::string, P> _duration_map
        ) : time_point_map_(_time_point_map), duration_map_(_duration_map) {}
            
        ~time_utils() { clear(); };
        
        inline void setTag(const std::string& _tag_name) {
            if (time_point_map_.contains(_tag_name)) {
                duration_map_.insert_or_assign(
                    _tag_name,
                    std::chrono::duration_cast<P>(
                        T::now() - time_point_map_[_tag_name]
                    )
                );
            } else {
                time_point_map_.emplace(
                    std::pair(_tag_name, T::now())
                );
            }
        }
            
        inline void displayInfo(const std::string& _tag_name) {
            std::cout << "[time_utils] Info '" << _tag_name << "' -> last set at: ";
            if (time_point_map_.contains(_tag_name)) {
                std::cout << time_point_map_[_tag_name].time_since_epoch().count() << ", ";
            } else {
                std::cout << "none, ";
            }
            auto duration { getDuration(_tag_name) };
            std::cout << "duration history: " << duration.count() << ", frequency: " <<  1.f / std::chrono::duration<float, std::ratio<1>>(duration).count();
        }
        
        inline auto getTag(const std::string& _tag_name) {
            return time_point_map_.contains(_tag_name)
                ? time_point_map_[_tag_name]
                : T::now();
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
            
        inline bool eraseDuration(const std::string& _tag_name) {
            return duration_map_.contains(_tag_name)
                ? [&]() -> bool { duration_map_.erase(_tag_name); return true; }
                : false;
        }
            
        inline void clear() {
            time_point_map_.clear();
            duration_map_.clear();
        }
        
    private:
        std::map<std::string, std::chrono::time_point<T>> time_point_map_;
        std::map<std::string, P> duration_map_;
    };
}

void doSomeThing() {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.5s);
}

int main() {
    ubn::time_utils t_u;
    
    t_u.setTag("Clock 1");
    doSomeThing();
    t_u.setTag("Clock 1");
    
    t_u.displayInfo("Clock 1");
}

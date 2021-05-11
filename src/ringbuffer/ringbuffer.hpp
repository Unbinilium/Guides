/*
 * @name: ringbuffer.hpp
 * @namespace: ubn
 * @class: ringbuffer
 * @brief: Simple ringbuffer implementation
 * @author Unbinilium
 * @version 1.0.1
 * @date 2021-05-12
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace ubn {
    template<typename T, const int64_t capacity>
    class ringbuffer {
    public:
        inline ringbuffer(void) { if (capacity < 1) throw std::invalid_argument("ringbuffer capacity < 1"); }
        
        inline bool push_head(const T& v) noexcept {
            m_data[++m_position % capacity] = v;
            if (m_capacity) { --m_capacity; }
            
            return m_capacity ? false : true;
        }
        
        template<typename T_ = T>
        inline bool push_head(T&& v, typename std::enable_if<!std::is_reference<T_>::value, std::nullptr_t>::type = nullptr) noexcept {
            m_data[++m_position % capacity] = std::move(v);
            if (m_capacity) { --m_capacity; }
            
            return m_capacity ? false : true;
        }
        
        inline T&& catch_tail(void) noexcept {
            if (m_position < capacity) {
                T* defalut { new T };
                
                return std::move(*defalut);
            }
            
            return std::move(m_data[(m_position - capacity + 1) % capacity]);
        }
    
    private:
        T       m_data[capacity];
        int64_t m_position { -1 };
        int64_t m_capacity { capacity + 1 };
    };
}

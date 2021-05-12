/*
 * @name: ringbuffer.hpp
 * @namespace: ubn
 * @class: ringbuffer
 * @brief: Simple ringbuffer implementation
 * @author Unbinilium
 * @version 2.0.0
 * @date 2021-05-12
 */

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

namespace ubn {
    template<typename T, const int64_t capacity>
    class ringbuffer {
    public:
        inline ringbuffer(void) { static_assert(capacity >= 1LL, "ringbuffer capacity < 1"); }
        
        inline bool push_head(const T& v) noexcept {
            m_buffer[++m_position % capacity] = v;
            return this->is_filled();
        }
        
        template<typename T_ = T>
        inline bool push_head(T&& v, typename std::enable_if<!std::is_reference<T_>::value, std::nullptr_t>::type = nullptr) noexcept {
            m_buffer[++m_position % capacity] = std::move(v);
            return this->is_filled();
        }
        
        inline T catch_tail(void) noexcept {
            return m_buffer[m_position + 1 < capacity ? capacity : (m_position + 1 + (m_capacity != capacity ? m_capacity++ : capacity) - capacity) % capacity];;
        }
        
        inline bool is_empty(void) noexcept {
            return m_capacity != capacity ? false : true;
        }
        
        inline void empty(void) noexcept {
            m_capacity = capacity;
            m_position = -1;
        }
        
    protected:
        inline bool is_filled(void) noexcept {
            return (m_capacity ? --m_capacity : 0) ? false : true;
        }
        
    private:
        T       m_buffer   [ capacity + 1 ];
        int64_t m_capacity { capacity };
        int64_t m_position { -1 };
    };
}

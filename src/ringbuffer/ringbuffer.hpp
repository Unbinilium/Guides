/*
 * @name: ringbuffer.hpp
 * @namespace: ubn
 * @class: ringbuffer
 * @brief: Simple ringbuffer implementation
 * @author Unbinilium
 * @version 1.0.2
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
            m_buffer[++m_position % capacity] = v;
            return ringbuffer::is_filled();
        }
        
        template<typename T_ = T>
        inline bool push_head(T&& v, typename std::enable_if<!std::is_reference<T_>::value, std::nullptr_t>::type = nullptr) noexcept {
            m_buffer[++m_position % capacity] = std::move(v);
            return ringbuffer::is_filled();
        }
        
        inline T catch_tail(void) noexcept {
            return m_buffer[m_position < capacity ? capacity : (m_position - capacity + 1) % capacity];
        }
        
    protected:
        inline bool is_filled(void) {
            if (m_capacity) { --m_capacity; }
            return m_capacity ? false : true;
        }
        
    private:
        T       m_buffer[capacity + 1];
        int64_t m_position { -1 };
        int64_t m_capacity { capacity + 1 };
    };
}

/*
 * @name: ringbuffer.hpp
 * @namespace: ubn
 * @class: ringbuffer
 * @brief: Simple FIFO ringbuffer implementation
 * @author Unbinilium
 * @version 1.0.0
 * @date 2021-05-12
 */

#pragma once

#include <cstdint>
#include <utility>
#include <type_traits>

namespace ubn {
    template<typename T, const int64_t capacity>
    class ringbuffer {
    public:
        /*
         * @brief: Initialize ringbuffer class
         * @param: typename, the typename of item(s) for ringbuffer to store
         * @param: const int64_t, the capacity of ringbuffer, overwrites the old item(s) when pushing if ringbuffer is full
         */
        inline ringbuffer(void) {
            static_assert(capacity >= 1LL, "ringbuffer capacity < 1");
        }
        
        /*
         * @brief:  Pushing an new item on ringbuffer head, return if it overwrites an item
         * @param:  const T&, const lvalue reference of typename T
         * @return: bool, whether you're pushing on a full ringbuffer which performs the overwrite action on an old item
         */
        inline bool push_head(const T& v) noexcept {
            m_buffer[++m_position % capacity] = v;
            return is_overwrite();
        }
        
        /*
         * @brief:  Pushing an new item on ringbuffer head, return if it overwrites an item
         * @param:  T&&, rvalue reference of typename T
         * @return: bool, whether you're pushing on a full ringbuffer which performs the overwrite action on an old item
         */
        template<typename T_ = T>
        inline bool push_head(T&& v, typename std::enable_if<!std::is_reference<T_>::value, std::nullptr_t>::type = nullptr) noexcept {
            m_buffer[++m_position % capacity] = std::forward<T>(v);
            return is_overwrite();
        }
        
        /*
         * @brief:  Get an item from ringbuffer tail, return the default initialized type T if ringbuffer is empty, and keeps the buffer in the ringbuffer if there had no overwrite action has been performed
         * @return: T, the item from ringbuffer tail or the default initialed type T if ringbuffer is empty
         */
        inline T catch_tail(void) noexcept {
            return m_buffer[is_empty() ? capacity : (m_position + (m_capacity != capacity ? m_capacity++ : capacity) - capacity + 1) % capacity];
        }
        
        /*
         * @brief:  Get current catchable item counts from ringbuffer
         * @return: int64_t, the current catchable item counts from ringbuffer
         */
        inline int64_t size(void) noexcept {
            return capacity - m_capacity;
        }
        
        /*
         * @brief:  Check if ringbuffer is empty, empty for true, otherwise for false
         * @return: bool
         */
        inline bool is_empty(void) noexcept {
            return m_capacity != capacity ? false : true;
        }
        
        /*
         * @brief:  Check if ringbuffer is full, full for true, otherwise for false
         * @return: bool
         */
        inline bool is_full(void) noexcept {
            return m_capacity ? false : true;
        }
        
        /*
         * @brief: Empty all buffer(s) inside ringbuffer, not free the memory, only reset the ringbuffers pointer position
         */
        inline void empty(void) noexcept {
            m_capacity = capacity;
            m_position = -1;
        }
        
    protected:
        inline bool is_overwrite(void) noexcept {
            return (m_capacity ? --m_capacity : 0) ? false : true;
        }
        
    private:
        T       m_buffer   [ capacity + 1 ];
        int64_t m_capacity { capacity };
        int64_t m_position { -1 };
    };
}

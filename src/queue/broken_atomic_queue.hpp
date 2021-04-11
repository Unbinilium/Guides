#pragma once

#include <queue>

#include <atomic>

#include <concepts>
#include <optional>

namespace ubn
{
    template <typename T>
    class queue
    {
    public:
        inline explicit queue(void) : m_data{}, m_size{m_data.size()}, m_lock{ATOMIC_FLAG_INIT}, m_poll_lock{ATOMIC_FLAG_INIT}
        {
            m_lock.test_and_set(std::memory_order::seq_cst);
        }

        inline ~queue(void) noexcept {}

        inline queue &operator=(const queue &) = delete;

        inline void push(std::convertible_to<T> auto &&v) noexcept
        {
            m_lock.wait(false, std::memory_order::acquire);
            m_lock.clear(std::memory_order::acquire);

            if (m_data.empty())
            {
                m_poll_lock.test_and_set(std::memory_order::release);
                m_poll_lock.notify_one();
            }

            m_data.emplace(std::forward<decltype(v)>(v));

            m_lock.test_and_set(std::memory_order::release);
            m_lock.notify_one();
        }

        inline T poll(void) noexcept
        {
            m_poll_lock.wait(false, std::memory_order::acquire);
            m_poll_lock.clear(std::memory_order::acquire);

            std::optional<T> tmp_;

            m_lock.wait(false, std::memory_order::acquire);
            m_lock.clear(std::memory_order::acquire);

            tmp_ = std::move(m_data.front());
            m_data.pop();

            if (!m_data.empty())
            {
                m_poll_lock.test_and_set(std::memory_order::release);
                m_poll_lock.notify_one();
            }

            m_lock.test_and_set(std::memory_order::release);
            m_lock.notify_one();

            return std::move(*tmp_);
        }

        inline std::size_t size(void) noexcept
        {
            m_lock.wait(false, std::memory_order::acquire);
            m_lock.clear(std::memory_order::acquire);

            m_size = m_data.size();

            m_lock.test_and_set(std::memory_order::release);
            m_lock.notify_one();

            return std::move(m_size);
        }

    private:
        std::queue<T> m_data;

        std::size_t m_size;

        std::atomic_flag mutable m_lock;
        std::atomic_flag mutable m_poll_lock;
    };
}

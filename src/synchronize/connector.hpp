#pragma once

#include <queue>

#include <atomic>

#include <cstddef>
#include <concepts>
#include <optional>

namespace ubn
{
    template <typename T>
    class connector
    {
    public:
        inline explicit connector(void) {}

        inline ~connector(void) noexcept {}

        inline connector &operator=(const connector &) = delete;

        inline void push(std::convertible_to<T> auto &&v) noexcept
        {
            m_flag.wait(true, std::memory_order::relaxed);

            m_data.emplace(std::forward<decltype(v)>(v));

            m_flag.test_and_set(std::memory_order::acquire);
            m_flag.notify_one();
        }

        inline T poll(void) noexcept
        {
            std::optional<T> tmp_;

            m_flag.wait(false, std::memory_order::relaxed);
            m_flag.clear(std::memory_order::release);
            tmp_ = std::move(m_data.front());
            m_data.pop();
            m_flag.notify_one();

            return std::move(*tmp_);
        }

    private:
        std::queue<T> m_data;

        alignas(2 * sizeof(std::max_align_t)) mutable std::atomic_flag m_flag{ATOMIC_FLAG_INIT};
    };
}

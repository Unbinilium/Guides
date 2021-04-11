#pragma once

#include <queue>

#include <mutex>
#include <condition_variable>

#include <concepts>
#include <optional>

namespace ubn
{

    template <typename T>
    class queue
    {
    public:
        inline explicit queue(void) : m_data{}, m_mutex{}, m_cv{} {}

        inline ~queue(void) noexcept {}

        inline queue &operator=(const queue &) = delete;

        inline void push(std::convertible_to<T> auto &&v) noexcept
        {
            std::lock_guard<std::mutex> lock_(m_mutex);

            m_data.emplace(std::forward<decltype(v)>(v));
            m_cv.notify_one();
        }

        inline T poll(void) noexcept
        {
            std::optional<T> tmp_;
            std::unique_lock<std::mutex> lock_(m_mutex);
            while (m_data.empty())
            {
                m_cv.wait(lock_);
            }

            tmp_ = std::move(m_data.front());
            m_data.pop();

            return std::move(*tmp_);
        }

        inline std::size_t size(void) noexcept
        {
            std::unique_lock<std::mutex> lock_(m_mutex);

            return m_data.size();
        }

    private:
        std::queue<T> m_data;

        std::mutex mutable m_mutex;
        std::condition_variable mutable m_cv;
    };

}

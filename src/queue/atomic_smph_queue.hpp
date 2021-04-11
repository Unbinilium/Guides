#pragma once

#include <queue>

#include <atomic>
#include <semaphore>

#include <cstdint>
#include <cstddef>

#include <concepts>
#include <optional>

namespace ubn
{
    template <typename T>
    class queue
    {
    public:
        inline explicit queue(void) {}

        inline ~queue(void) noexcept {}

        inline queue &operator=(const queue &) = delete;

        inline void push(const T &v) noexcept
        {
            lock_();
            m_data.emplace(v);
            unlock_();

            m_avaliable.release();
        }

        inline void push(std::convertible_to<T> auto &&v) noexcept
        {
            lock_();
            m_data.emplace(std::forward<decltype(v)>(v));
            unlock_();

            m_avaliable.release();
        }

        inline T poll(void) noexcept
        {
            std::optional<T> tmp_;

            m_avaliable.acquire();

            lock_();
            tmp_ = std::move(m_data.front());
            m_data.pop();
            unlock_();

            return std::move(*tmp_);
        }

        inline std::size_t size(void) noexcept
        {
            lock_();
            const std::size_t tmp_{m_data.size()};
            unlock_();

            return std::move(tmp_);
        }

    protected:
        inline void lock_(void) noexcept
        {
            auto const ticket_{m_in.fetch_add(1, std::memory_order::acquire)};
            while (true)
            {
                auto const now_{m_out.load(std::memory_order::acquire)};
                if (now_ == ticket_)
                    return;
                m_out.wait(now_, std::memory_order::relaxed);
            }
        }

        inline void unlock_(void) noexcept
        {
            m_out.fetch_add(1, std::memory_order::release);
            m_out.notify_all();
        }

    private:
        std::queue<T> m_data;

        std::counting_semaphore<SSIZE_MAX> m_avaliable{0};

        static constexpr std::size_t hardware_destructive_interference_size{2 * sizeof(std::max_align_t)};

        alignas(hardware_destructive_interference_size) mutable std::atomic<std::size_t> m_in;
        alignas(hardware_destructive_interference_size) mutable std::atomic<std::size_t> m_out;
    };
}

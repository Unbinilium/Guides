#pragma once

#include <atomic>

#include <memory>
#include <limits>
#include <ranges>

#include <cstdint>
#include <cstddef>

#include <concepts>
#include <optional>

namespace ubn {
    static constexpr std::size_t hardware_constructive_interference_size { 2 * sizeof(std::max_align_t) };

    template <typename T>
    class queue {
    public:
        inline explicit queue(const std::size_t capacity = UINT16_MAX) :
            m_capacity  { capacity },
            m_allocator { std::allocator<container<T>>() },
            m_front     { ATOMIC_VAR_INIT(0) },
            m_back      { ATOMIC_VAR_INIT(0) } {
                
            m_containers = m_allocator.allocate(m_capacity + 1);

            if  (reinterpret_cast<std::size_t>(m_containers) % alignof(container<T>)) m_allocator.deallocate(m_containers, m_capacity + 1);
            for (auto i : std::views::iota(0u, m_capacity))                           new (&m_containers[i]) container<T>();
        }

        inline ~queue(void) noexcept {
            for (auto i : std::views::iota(0u, m_capacity)) m_containers[i].~container();
            m_allocator.deallocate(m_containers, m_capacity + 1);
        }

        inline queue           (const queue&) = delete;
        inline queue &operator=(const queue&) = delete;

        inline void push(const T& v)                      noexcept { emplace(v); }
        inline void push(std::convertible_to<T> auto&& v) noexcept { emplace(std::forward<decltype(v)>(v)); }

        inline T poll(void) noexcept {
            std::optional<T> tmp_;

            auto const tail_      { m_back.fetch_add(1, std::memory_order::acquire) };
            auto&      container_ { m_containers[m_index(tail_)] };
            auto const ticket_    { m_ticket(tail_) * 2 + 1 };
            while (true) {
                auto const now_ { container_.m_ticket_.load(std::memory_order::acquire) };
                if (now_ == ticket_) break;
                container_.m_ticket_.wait(now_, std::memory_order::relaxed);
            }
            tmp_ = container_.move();
            container_.destruct();
            container_.m_ticket_.store(m_ticket(tail_) * 2 + 2, std::memory_order::release);
            container_.m_ticket_.notify_all();

            return std::move(*tmp_);
        }

    protected:
        template <typename... Args>
        inline void emplace(Args&&... args) noexcept {
            auto const head_      { m_front.fetch_add(1, std::memory_order::acquire) };
            auto&      container_ { m_containers[m_index(head_)] };
            auto const ticket_    { m_ticket(head_) * 2 };
            while (true) {
                auto const now_ { container_.m_ticket_.load(std::memory_order::acquire) };
                if (now_ == ticket_) break;
                container_.m_ticket_.wait(now_, std::memory_order::relaxed);
            }
            container_.construct(std::forward<Args>(args)...);
            container_.m_ticket_.store(m_ticket(head_) * 2 + 1, std::memory_order::release);
            container_.m_ticket_.notify_all();
        }

        template <typename V>
        struct container {
        public:
            inline ~container    (void)           noexcept { if (m_ticket_.load(std::memory_order::acquire)) destruct(); }

            template <typename... Args>
            inline void construct(Args&&... args) noexcept { new (&m_storage_) V(std::forward<Args>(args)...); }
            inline void destruct (void)           noexcept { reinterpret_cast<V*>(&m_storage_)->~V(); }
            inline V&&  move     (void)           noexcept { return reinterpret_cast<V&&>(m_storage_); }

            alignas(hardware_constructive_interference_size) std::atomic_size_t mutable m_ticket_ { 0 };

        private:
            typename std::aligned_storage<sizeof(V), alignof(V)>::type m_storage_;
        };

    private:
        constexpr std::size_t m_index (const std::size_t i) const noexcept { return reinterpret_cast<std::size_t>(i % m_capacity); }
        constexpr std::size_t m_ticket(const std::size_t i) const noexcept { return reinterpret_cast<std::size_t>(i / m_capacity); }

        container<T>*                m_containers;
        std::size_t const            m_capacity;
        std::allocator<container<T>> m_allocator [[no_unique_address]];
        
        alignas(hardware_constructive_interference_size) std::atomic_size_t mutable m_front;
        alignas(hardware_constructive_interference_size) std::atomic_size_t mutable m_back;
    };
}

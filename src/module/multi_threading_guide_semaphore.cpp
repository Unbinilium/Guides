#include <iostream>

#include <mutex>
#include <thread>
#include <chrono>

#include <semaphore>

#include <atomic>
#include <memory>
#include <vector>

#include <random>

struct Object {
    int               num         { 0 };
    std::atomic<bool> is_processed{ false };
    std::atomic<bool> is_staled   { false };
};

struct Container {
    std::mutex                           container_mutex;
    std::mutex                           smph_mutex;
    std::binary_semaphore                smph{ 0 };
    std::vector<std::shared_ptr<Object>> obj_vec;
};

static inline void delay(const double& r_l, const double& r_r) {
    std::mt19937_64 gen{ std::random_device{}() };
    std::uniform_int_distribution<> dist{ static_cast<int>(r_l),static_cast<int>(r_r) };
    std::this_thread::sleep_for(std::chrono::microseconds{ dist(gen) });
}

static inline bool stream(std::shared_ptr<Object>& p_obj) {
    static std::atomic<int> a{ 0 };
    std::cout << "gen: " << ++a << std::endl;

    p_obj->num = std::move(a);

    return true;
}

static inline void helper(std::unique_ptr<Container>& p_container, std::shared_ptr<Object>& p_obj, const double& delay_us) {
    static std::mutex                            op_mutex;
    static std::chrono::steady_clock::time_point time_stamp;

    std::lock_guard<std::mutex> op_guard(op_mutex);
    std::this_thread::sleep_until(std::chrono::duration<double, std::micro>(delay_us) + time_stamp);

    {
        while (!stream(p_obj)) { std::this_thread::yield(); }

        p_container->container_mutex.lock();
        p_container->obj_vec.push_back(p_obj);
        p_container->container_mutex.unlock();
    }

    time_stamp = std::chrono::steady_clock::now();
}

static inline void process(std::shared_ptr<Object>& p_obj) {
    {
        std::cout << "pro: " << p_obj->num << std::endl;
    }

    delay(1e3, 1e4);
}

static inline void clean(std::unique_ptr<Container>& p_container) {
    std::lock_guard<std::mutex> container_guard(p_container->container_mutex);

    std::remove_if(p_container->obj_vec, [](std::shared_ptr<Object>& iter_obj) -> bool { return bool(iter_obj->is_staled); });

    {
        std::cout << "siz: " << p_container->obj_vec.size() << std::endl;
    }
}

static inline void final(std::shared_ptr<Object>& p_obj_prev, std::shared_ptr<Object>& p_obj_next) {
    {
        std::cout << "sum: " << p_obj_prev->num << "+" << p_obj_next->num << "=" << p_obj_prev->num + p_obj_next->num << std::endl;
    }

    delay(1e4, 1e5);
}

static inline void filter(std::unique_ptr<Container>& p_container) {
    static size_t  i;
    static size_t  obj_vec_size;
    static ssize_t obj_next_idx;

    p_container->container_mutex.lock();
    obj_vec_size = p_container->obj_vec.size();
    p_container->container_mutex.unlock();

    obj_next_idx = -1;

    for (i = 0; i != obj_vec_size; ++i) {
        if (p_container->obj_vec[i]->is_processed) { ++obj_next_idx; }
        else { break; }
    }

    if (obj_next_idx > 0) {
        {
            final(p_container->obj_vec[0], p_container->obj_vec[obj_next_idx]);
        }

        for (i = 0; i != obj_next_idx; ++i) { p_container->obj_vec[i]->is_staled = true; }
    }
}

int main() {
    using namespace std::literals;
    using clock = std::chrono::steady_clock;

    std::unique_ptr<Container> container(new Container);

    const auto begin = clock::now();
    {
        std::vector<std::thread> threads;

        for (size_t i = 1; i != std::thread::hardware_concurrency(); ++i) {
            threads.emplace_back([&m_container = container]() mutable -> void {
                    for (;;) {
                        std::shared_ptr<Object> p_obj(new Object);

                        {
                            helper(m_container, p_obj, 1e3);
                            process(p_obj);
                        }

                        p_obj->is_processed = true;
                        m_container->smph.release();
                    }
                }
            );
        }
        threads.emplace_back([&m_container = container]() mutable -> void {
                for (;;) {
                    m_container->smph.acquire();
                    std::lock_guard<std::mutex> smph_guard(m_container->smph_mutex);

                    std::cout << "sem: notified!" << std::endl;

                    {
                        filter(m_container);
                        clean(m_container);
                    }
                }
            }
        );

        for (auto& thread : threads) { thread.join(); }
    }
    const auto end = clock::now();
    std::cout << "Time elapsed " << (end - begin) / 1.0ms << "ms" << std::endl;

    return 0;
}

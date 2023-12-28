#pragma once
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <boost/fiber/all.hpp>
#include <boost/fiber/detail/thread_barrier.hpp>
#include <vector>
#include <type_traits>
#include <chrono>

namespace async {


inline auto suspend_exec() -> void {
    boost::this_fiber::yield();
}

template<typename T> 
concept Duration = std::is_same_v<T, std::chrono::seconds> ||
    std::is_same_v<T, std::chrono::microseconds> ||
    std::is_same_v<T, std::chrono::milliseconds> ||
    std::is_same_v<T, std::chrono::minutes> ||
    std::is_same_v<T, std::chrono::hours>;


template<Duration TO>
inline auto sleep_for(TO to) -> void {
    boost::this_fiber::sleep_for(to);
}

template<typename Clock, typename Duration>
inline auto sleep_until(const std::chrono::time_point<Clock, Duration>& to) -> void {
    boost::this_fiber::sleep_until(to);
}

class work_stealing_pool {
public:
    explicit work_stealing_pool(std::size_t tc = std::thread::hardware_concurrency());

        ~work_stealing_pool() {
            stop();
        }

        work_stealing_pool(const work_stealing_pool&) = delete;
        work_stealing_pool& operator = (const work_stealing_pool&) = delete;

        auto stop() -> void;

        auto wait() -> void;

        auto no_tasks() const -> bool {
            return counter.empty();
        }

        // auto notify_done() -> void {
        //     run = false;
        // }

        template<typename F>
        requires std::is_invocable_v<F>
        auto post(F&& operation) -> void {
            boost::fibers::fiber([this, f = std::move(operation)] () {
                ++counter;
			    f();
                --counter;
            }).detach();
        }

        auto sync() -> void {
            tasks_sync.wait();
        }
        
private:
    auto start_background() -> void;

    // auto keep_running() const -> bool {
    //     if (!run) { 
    //         return active_fiber > 0;
    //     }
    //     return run;
    // }
    

private:
    struct task_counter {
        std::atomic_uint64_t counter{0};
        boost::fibers::condition_variable_any notifier;
        std::mutex count_sync;

        auto operator ++ () -> task_counter& {
            ++counter;
            return *this;
        }

        auto operator -- () -> task_counter& {
            lock_type lock(count_sync);               
            if (--counter == 0) { // Decrement fiber counter for each completed fiber.
                lock.unlock();
                notifier.notify_all(); // Notify all fibers waiting.
            }
            return *this;
        }

        auto empty() const -> bool {
            return counter == 0;
        }

        auto sync() {
            lock_type lock(count_sync);
            notifier.wait(lock, [this]() { 
                return empty(); 
            });
        }
    };

    struct run_await {
        std::atomic_bool run{false};
        boost::fibers::condition_variable_any notifier;
        std::mutex count_sync;

        auto sync() {
            lock_type lock(count_sync);
            notifier.wait(lock, [this]() { 
                return run == false;
            });
        }

        auto notify() {
            run = false;
            notifier.notify_all(); 
        }
    };
private:
    using lock_type = std::unique_lock<std::mutex>;
    using workers_t = std::vector<std::thread>;

    //std::atomic_bool run{false};
    // std::atomic_uint64_t active_fiber{0};
    // std::mutex count_sync;
    // boost::fibers::condition_variable_any fiber_count_cv;
    boost::fibers::detail::thread_barrier tasks_sync;
    task_counter counter;
    run_await    run;
    workers_t    workers;

};	// end of pool class

}	// end of async namespace


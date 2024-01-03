#pragma once
#include <cstddef>
#include <thread>
#include <boost/fiber/all.hpp>
#include <boost/fiber/detail/thread_barrier.hpp>
#include <vector>
#include <type_traits>

namespace async {

class task_executer {
public:
    explicit task_executer(std::size_t tc = std::thread::hardware_concurrency());

        ~task_executer() {
            stop();
        }

        task_executer(const task_executer&) = delete;
        task_executer& operator = (const task_executer&) = delete;

        auto stop() -> void;

        template<typename F>
        requires std::is_invocable_v<F>
        auto post(F&& operation) -> void {
            boost::fibers::fiber([this, f = std::move(operation)] () {
                ++counter;
			    f();
                --counter;
            }).detach();
        }
        
private:
    auto start_background() -> void;

private:
    struct tasks_counter {
        auto operator ++ () -> tasks_counter& {
            ++counter;
            return *this;
        }

        auto operator -- () -> tasks_counter& {
            std::unique_lock<std::mutex> lock{guard};
            --counter;
            if (counter == 0) {
                lock.unlock();
                cv.notify_all();
            }
            return *this;
        }

        auto wait() -> void {
            std::unique_lock<std::mutex> lock{guard};
            cv.wait(lock, [this] {
                return counter == 0;
            });
        }

        std::atomic_uint64_t                    counter;
        std::mutex                              guard;
        boost::fibers::condition_variable_any   cv;
    };

private:
    using lock_type = std::unique_lock<std::mutex>;
    using workers_t = std::vector<std::thread>;

    boost::fibers::condition_variable_any tasks_sync;
    boost::fibers::mutex tasks_guard;
    tasks_counter counter;
    workers_t    workers;

};	// end of pool class

}	// end of async namespace


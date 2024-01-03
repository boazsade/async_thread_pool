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
        auto post(F&& operation) ->  boost::fibers::future<typename std::result_of<F()>::type> {
            using result_type = typename std::result_of<F()>::type;
            
            if constexpr (std::is_void_v<result_type>) {
                return invoke_void_f(std::move(operation));
            } else {
                return invoke_other(std::move(operation));
            }
        }
        
private:
    auto start_background() -> void;

    template<typename result_type, typename F>
    auto invoke_op(F&& op) -> boost::fibers::future<result_type> {
        using future_type = boost::fibers::future<result_type>;

        boost::fibers::packaged_task<result_type()> pt(std::move(op));

        future_type result{pt.get_future()};
        boost::fibers::fiber { 
            std::move(pt) 
        }.detach();

        return result;
    }

    template<typename F>
    auto invoke_void_f(F&& operation) -> boost::fibers::future<void> {
        auto wrapper = [this, op = std::move(operation)]() {
            ++counter;
            op();
            --counter;
        };

        return invoke_op<void>(std::move(wrapper));
    }

    template<typename F>
    auto invoke_other(F&& operation) -> boost::fibers::future<typename std::result_of<F()>::type> {
        auto wrapper = [this, op = std::move(operation)]() {
            ++counter;
            auto r = op();
            --counter;
            return r;
        };

        return invoke_op<typename std::result_of<F()>::type>(std::move(wrapper));
    }

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


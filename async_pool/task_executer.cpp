#include "task_executer.h"

namespace async {

task_executer::task_executer(std::size_t tc) {
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    for (auto i = 0lu; i < tc; i++) {
        workers.emplace_back(std::thread([this] () {
            this->start_background();
        }));
    }
}

auto task_executer::stop() -> void {
    counter.wait();
    tasks_sync.notify_all();
    for (auto&& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}

auto task_executer::start_background() -> void {
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    tasks_guard.lock();
    tasks_sync.wait(tasks_guard);
    tasks_guard.unlock();
    counter.wait();
}
    
} // namespace async 


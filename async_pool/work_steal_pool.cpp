#include "work_steal_pool.h"

namespace async {

work_stealing_pool::work_stealing_pool(std::size_t tc)  :
        //run{true},
        tasks_sync{tc + 1} {
    boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
    for (auto i = 0lu; i < tc; i++) {
        workers.emplace_back(std::thread([this] () {
            this->start_background();
        }));
    }
}

auto work_stealing_pool::wait() -> void  {
    // lock_type lock(count_sync);
    // fiber_count_cv.wait(lock, [this]() { 
    //     return no_tasks(); 
    // });
    counter.sync();
}

auto work_stealing_pool::stop() -> void {
    //run = false;    
    run.notify();
    wait();
    for (auto&& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}

auto work_stealing_pool::start_background() -> void {
    boost::fibers::use_scheduling_algorithm< boost::fibers::algo::shared_work >();
    tasks_sync.wait();
    // lock_type lock(count_sync);
    // while (keep_running()) {
    //     fiber_count_cv.wait(lock, [this]() {
    //         return no_tasks();
    //     });
    // }
    run.sync();     // wait to be notify to stop running
    counter.sync(); // ensure that we are not leaving anything behind
}
    
} // namespace async 


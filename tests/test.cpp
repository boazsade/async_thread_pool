#include "async_pool/work_steal_pool.h"
#include <iostream>
#include <optional>
#include <sstream>


auto just_because(int me) -> void {
    auto my_thread = std::this_thread::get_id();
    auto print_details = [me, my_thread](const char* msg) -> void {
	    std::ostringstream buffer;
        buffer << "fiber " << me << msg << my_thread << '\n';
        std::cout << buffer.str() << std::flush;
        (void)msg;
    };

    try {    	
	    print_details(" started on thread ");
        for ( auto i = 0; i < 100; ++i) { /*< loop ten times >*/
            async::suspend_exec();
            auto new_thread = std::this_thread::get_id();
            if ( new_thread != my_thread) {
                my_thread = new_thread;
                print_details(" switched to thread ");
            }
        }
    } catch ( ... ) {
        print_details( "we have some error! ");
    }
}

auto main(int argc , char** argv) -> int {
    auto max = 1'000;
    if (argc > 1) {
	    max = std::atoi(argv[1]);
    }
    std::cout << "generating " << max << " subtasks for this" << std::endl;
    auto generate = [max]() mutable  -> std::optional<int> {
	    if (max > 0) {
		    return --max;
	    }
    	return {};
    };

    std::cout << "main thread started " << std::this_thread::get_id() << std::endl;
    async::work_stealing_pool pool;

    for (auto g = generate(); g.has_value(); g = generate()) {
        pool.post([val = *g]() {
            just_because(val);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "waiting to sync with other threads" << std::endl;
    pool.sync();
    std::cout << "finish executing all tasks\n";

    // we should see that the tasks are not running here
    std::cout << "waiting for the tasks to complete" << std::endl;
    std::cout << "finish waiting for all the tasks" << std::endl;
    pool.stop();
    std::cout << "we are done with all of it" <<std::endl;
}
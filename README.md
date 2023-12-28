# async_thread_pool
This is a small boost fiber based thread pool that allow to execute tasks (on fiber) in multi worker threads

## Work Stealing Pool
This pool is based on the idea that the tasks can move between threads.
The main flow:
- Start a new task that execute a work.
- The task in dispatched by one of the available threads.
- If the task in suspended, than when it will be resumed, it may move to other thread, based on which thread can execute the ready task.
- The pool can go down once we don't have any tasks running.
A simple usage of this is:
```cpp
#include "async_pool/work_steal_pool.h"
#include <chrono>
#include <iostream>

auto my_work(int a, int b, double d) -> void {
    if (b < a) {
        return;
    }
    while (a < b) {
        auto s = dispatch(d);
        async::sleep_for(1s);
        a += s;
    }
}

auto main(int, char**) -> int {
    async::work_stealing_pool pool;
    for (auto i = 0; i < 1'000; i++) {
        pool.post([]() {
            my_work(i, i + i * 2 + 5);
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "waiting to sync with other threads" << std::endl;
    pool.sync();    // just let the other threads in the pool do something..
    std::cout << "finish executing all tasks\n";
    td::cout << "waiting for the tasks to complete" << std::endl;
    std::cout << "finish waiting for all the tasks" << std::endl;
    pool.stop();    // and stop, since we don't have more work to commit
    std::cout << "we are done with all of it" <<std::endl;
}

```
Note that we need to run one of the function: sleep_for, sleep_until or suspend_exec otherwise the tasks will not be switched over, this to simulate async execution.
If the code is not I/O bound, there is no much use of this pattern as we are not really switching between tasks, and a regular thread pool that run every function to its completion will be more suitable.
Also note that right now this code do not support return value from a task, nor it is able to accept a function that is accepting any arguments. This limitation from the API can overcome with calling a lambda function.
You can also use other means such as std::function or std::bind. Please note that since the parameter are passed to some other execution context you must not pass them by reference. In C++ there is no direct way to check this, (something like Rust's [call by value receiver](https://doc.rust-lang.org/std/ops/trait.FnOnce.html)). Later versions will support the return type as well as processing multiple arguments functions.
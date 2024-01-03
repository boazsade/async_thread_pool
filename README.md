# async_thread_pool
This is a small boost fiber based thread pool that allow to execute tasks (on fiber) in multi worker threads

## Task Execution
This pool is based on the idea that the tasks can move between threads.
The main flow:
- Start a new task that execute a work.
- The task in dispatched by one of the available threads.
- If the task in suspended, than when it will be resumed, it may move to other thread, based on which thread can execute the ready task.
- The pool can go down once we don't have any tasks running.
A simple usage of this is:
```cpp
#include "async_pool/task_executer.h"
#inclde "someother_task_based_code.h"
#include <iostream>
#include <chono>

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
    async::task_executer pool;
    for (auto i = 0; i < 1'000; i++) {
        pool.post([]() {
            my_work(i, i + i * 2 + 5);
        });
    }

    std::cout << "finish committing all tasks\n";
    pool.stop();    // this is not really required, but this is how we will see the next printout
    std::cout << "we are done with all of it" <<std::endl;
}

```
Note that we need to run one of the function: sleep_for, sleep_until or suspend_exec otherwise the tasks will not be switched over, this to simulate async execution.
If the code is not I/O bound, there is no much use of this pattern as we are not really switching between tasks, and a regular thread pool that run every function to its completion will be more suitable.
Also note that right now this code do not support return value from a task, nor it is able to accept a function that is accepting any arguments. This limitation from the API can overcome with calling a lambda function.
You can also use other means such as std::function or std::bind. Please note that since the parameter are passed to some other execution context you must not pass them by reference. 
In C++ there is no direct way to check this, (something like Rust's [call by value receiver](https://doc.rust-lang.org/std/ops/trait.FnOnce.html)).
### Return from executer
This task executor support return value from its context.
Note that this is a lazy return value, since we are running a task and not a normal function.
To extract the result from the function, you would need to call:
```
result.get()
```
on the result from the executer.
Please note that by calling this, you would be blocked, so do so once you are ready to be blocked.
```cpp
#include "async_pool/task_executer.h"
#include <iostream>
#include <vector>

auto my_task(int a, int b, int c) -> int {
    while (a + b < c) {
        a++;
        b++;
        async::sleep_for(1s);
    }
    return c * a + b;
}

auto main(int, char**) -> int {
    async::task_executer pool{tc};
    std::vector<async::future_type<std::uint64_t>> counters;
    for (auto i = 5; i < 50; i++) {
        counters.push_back(std::move(pool.post([a = i, b = i + 4, c = i * 5]() {
            return my_task(a, b, c);
        })));
    }
    auto total_count = std::accumulate(std::begin(counters), std::end(counters), 0, [](auto&& count, auto&& f) {
        return count + f.get();
    });
    std::cout << "We accumulated " << total_count << " of results from all the tasks\n";
    pool.stop();
}

```

#include "async_pool/task_executer.h"
#include "async_pool/pool_utils.h"
#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>
#include <iterator>

std::atomic_uint64_t total_count = 0;

auto get_rand_letter() -> char {
    static constexpr std::string_view letters = "abcdefghijklmnopqrspuvwzyz";
    return letters[rand() % (letters.size() - 1)];
}

auto just_because(int me, std::string_view fn) -> void {
    auto my_thread = std::this_thread::get_id();
    auto end = std::istream_iterator<char>();
    auto print_details = [me, my_thread](const char* msg) -> void {
	    std::ostringstream buffer;
        buffer << "fiber " << me << msg << my_thread << '\n';
        std::cout << buffer.str() << std::flush;
        (void)msg;
    };

    auto find_in_file = [&end](char what, std::istream_iterator<char> curr) -> std::istream_iterator<char> {
        auto i = std::find_if(curr, end, [what](auto v) {
            return v == what;
        });
        return i == end ? end : ++i;
    };

    print_details(" started on thread ");

    try {
        std::ifstream input{fn.data()};
        if (!input) {
            throw std::runtime_error{"failed to open input file for reading"};
        }

        auto i = find_in_file(get_rand_letter(), std::istream_iterator<char>(input));
        
        
        while (i != end) {
            ++total_count;
            async::sleep_for(std::chrono::milliseconds(1));
            //async::suspend_exec();
            auto new_thread = std::this_thread::get_id();
            if (new_thread != my_thread) {
                my_thread = new_thread;
                print_details(" switched to thread ");
            }
            i = find_in_file(get_rand_letter(), i);
        }
    } catch ( ... ) {
        print_details( " we have some error! ");
    }
    print_details(" is done ");
}

auto main(int argc , char** argv) -> int {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << ": <file path> [iterations]\n";
        return -1;
    }

    auto max = (argc > 2) ? std::atoi(argv[2]) : 1'000;
    const auto tc = 40u;

    std::cout << "generating " << max << " subtasks for this" << std::endl;
    auto generate = [max]() mutable  -> std::optional<int> {
	    if (max > 0) {
		    return --max;
	    }
    	return {};
    };

    std::cout << "main thread started " << std::this_thread::get_id() << std::endl;
    async::task_executer pool{tc};
    
    for (auto g = generate(); g.has_value(); g = generate()) {
        pool.post([val = *g, fn = argv[1]]() {
            just_because(val, fn);
        });
    }
    pool.stop();
    std::cout << "we are done with all of it, we had " << total_count.load() << " found successful in the file " << argv[1] <<std::endl;
}
#pragma once
#include <boost/fiber/all.hpp>
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
}       // end of namespace async
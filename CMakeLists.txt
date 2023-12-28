cmake_minimum_required(VERSION 3.10.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(thread_pool CXX)


include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(async_pool)
add_subdirectory(tests)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
include(CPack)


# test support
enable_testing()

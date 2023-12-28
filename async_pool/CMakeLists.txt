include(flags)
include(dependencies)

add_library(async_pool STATIC
    work_steal_pool.h  work_steal_pool.cpp
)


target_include_directories(async_pool PUBLIC ../ )
enable_testing()
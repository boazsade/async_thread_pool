include(flags)
include(dependencies)

add_library(async_pool STATIC
    task_executer.h  task_executer.cpp
)


target_include_directories(async_pool PUBLIC ../ )
enable_testing()
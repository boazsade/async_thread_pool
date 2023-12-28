include(dependencies)
include(flags)
list(APPEND MAIN_FILES
    test.cpp
)

add_executable(test_pool  ${MAIN_FILES})
list(APPEND EXTRA_LIBS async_pool 
    Boost::coroutine Boost::thread 
    Boost::context Boost::fiber
    pthread dl)
list(APPEND EXTRA_INCLUDES ../ ./ )

target_include_directories(test_pool PUBLIC ${EXTRA_INCLUDES} )
target_link_libraries(test_pool PUBLIC ${EXTRA_LIBS})

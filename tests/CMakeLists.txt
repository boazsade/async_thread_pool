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


list(APPEND MAIN_FILES_WITH_RESULTS
    test_with_results.cpp
)

add_executable(test_with_result  ${MAIN_FILES_WITH_RESULTS})
list(APPEND EXTRA_INCLUDES ../ ./ )

target_include_directories(test_with_result PUBLIC ${EXTRA_INCLUDES} )
target_link_libraries(test_with_result PUBLIC ${EXTRA_LIBS})
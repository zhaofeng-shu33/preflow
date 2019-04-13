cmake_minimum_required(VERSION 3.5)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

find_package(lemon REQUIRED)
add_executable(test_mf main.cpp)

target_link_libraries(mf ${LEMON_LIBRARY})

if(WIN32)
    target_link_libraries(test_mf debug ${GTEST_MAIN_LIBRARY_DEBUG} debug ${GTEST_LIBRARY_DEBUG})
    target_link_libraries(test_mf optimized ${GTEST_MAIN_LIBRARY} optimized ${GTEST_LIBRARY})
elseif(UNIX)
    target_link_libraries(test_mf ${GTEST_MAIN_LIBRARY} ${GTEST_LIBRARY} pthread)
endif()  

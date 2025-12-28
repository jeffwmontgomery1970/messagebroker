cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
FetchContent_MakeAvailable(googletest)

find_package(Threads REQUIRED)

add_executable(messagebroker messagebroker.cpp)
target_link_libraries(messagebroker sbio-wrapper pthread)

add_library(broker SHARED messagebroker.cpp)
target_link_libraries(broker sbio-wrapper pthread)

enable_testing()
add_executable(messagebroker_tests test_messagebroker.cpp)
target_link_libraries(messagebroker_tests gtest_main sbio-wrapper broker pthread)
add_test(NAME messagebroker_tests COMMAND messagebroker_tests)

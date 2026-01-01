cmake_minimum_required(VERSION 3.10)
project(MessageBroker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# Option to build tests
option(BUILD_TESTS "Build unit tests" ON)

# Create shared library
add_library(pubsub SHARED
    subscriber.cpp
    subscriberlist.cpp
    publisher.cpp
)

# Set library properties
set_target_properties(pubsub PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "pubsub"
)

# Link libraries for shared library
target_link_libraries(pubsub
    Threads::Threads
    sbio-wrapper
)

# Create executable
add_executable(app main.cpp)

# Link libraries for executable
target_link_libraries(app
    pubsub
    Threads::Threads
    sbio-wrapper
)

# Build tests if enabled
if(BUILD_TESTS)
    # Find Google Test
    find_package(GTest REQUIRED)
    
    # Create test library with mock SBIO
    add_library(pubsub_test STATIC
        subscriber.cpp
        subscriberlist.cpp
        publisher.cpp
        tests/mock_sbio_wrapper.cpp
    )
    
    # Set include directories for test library
    target_include_directories(pubsub_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    
    target_link_libraries(pubsub_test
        Threads::Threads
    )
    
    # Create test executable
    add_executable(run_tests
        tests/test_main.cpp
        tests/test_publisher.cpp
        tests/test_subscriber.cpp
        tests/test_subscriberlist.cpp
        tests/mock_sbio_wrapper.cpp
    )
    
    # Link test executable
    target_link_libraries(run_tests
        pubsub_test
        GTest::gtest
        GTest::gmock
        Threads::Threads
    )
    
    # Set include directories for tests
    target_include_directories(run_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    
    # Enable testing
    enable_testing()
    add_test(NAME MessageBrokerTests COMMAND run_tests)
endif()

# Install targets
install(TARGETS pubsub
    LIBRARY DESTINATION lib
)

install(TARGETS app
    RUNTIME DESTINATION bin
)
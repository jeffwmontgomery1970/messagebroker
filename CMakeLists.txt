cmake_minimum_required(VERSION 3.14)
project(messagebroker CXX)

# Require at least C++17
cmake_minimum_required(VERSION 3.14)
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

# Path to Storyboard Engine libs (adjust if your installation differs)
set(STB_ENGINE_LIB_DIR "$ENV{HOME}/Crank_Software/Storyboard_Engine/9.0-stable.202509150854/linux-x86_64-opengles_2.0-x11-obj/lib")

# Prefer explicit library files from the Storyboard Engine location
find_library(SBIO_WRAPPER_LIB NAMES sbio-wrapper PATHS ${STB_ENGINE_LIB_DIR} NO_DEFAULT_PATH)
find_library(GRE_LIB NAMES gre libgre PATHS ${STB_ENGINE_LIB_DIR} NO_DEFAULT_PATH)

if(NOT SBIO_WRAPPER_LIB)
  message(FATAL_ERROR "Could not find libsbio-wrapper.so in ${STB_ENGINE_LIB_DIR}.\nSet STB_ENGINE_LIB_DIR to the correct path or install the library.")
endif()

if(NOT GRE_LIB)
  message(WARNING "Could not find libgre.so in ${STB_ENGINE_LIB_DIR}. Some symbols may remain unresolved at link time.")
endif()

add_executable(messagebroker messagebroker.cpp)
target_include_directories(messagebroker PRIVATE /usr/local/include)
target_link_libraries(messagebroker PRIVATE ${SBIO_WRAPPER_LIB} Threads::Threads)

add_library(broker SHARED messagebroker.cpp)
target_include_directories(broker PRIVATE /usr/local/include)
target_link_libraries(broker PRIVATE ${SBIO_WRAPPER_LIB} Threads::Threads)

# Make sure the runtime linker can locate Storyboard Engine libs during development
set_target_properties(messagebroker PROPERTIES
  BUILD_RPATH "${STB_ENGINE_LIB_DIR}"
  INSTALL_RPATH "${STB_ENGINE_LIB_DIR}"
  INSTALL_RPATH_USE_LINK_PATH TRUE
)
set_target_properties(broker PROPERTIES
  BUILD_RPATH "${STB_ENGINE_LIB_DIR}"
  INSTALL_RPATH "${STB_ENGINE_LIB_DIR}"
  INSTALL_RPATH_USE_LINK_PATH TRUE
)

enable_testing()
add_executable(messagebroker_tests test_messagebroker.cpp)
target_link_libraries(messagebroker_tests PRIVATE gtest_main ${SBIO_WRAPPER_LIB} broker Threads::Threads)
add_test(NAME messagebroker_tests COMMAND messagebroker_tests)

cmake_minimum_required(VERSION 3.23)

project(netkit VERSION 0.1.0 LANGUAGES C CXX)

option(NETKIT_BUILD_TESTS "Build netkit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(netkit_transport STATIC
    src/shared_memory_transport.cpp
    src/control_channel_backend.cpp
    src/shm_pipe_factory.cpp
)
add_library(NetKit::Transport ALIAS netkit_transport)

target_include_directories(netkit_transport
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(netkit_transport PUBLIC cxx_std_20)

if(NETKIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

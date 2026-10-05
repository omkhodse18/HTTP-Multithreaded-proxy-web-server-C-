cmake_minimum_required(VERSION 3.16)
project(ProxyServer LANGUAGES CXX)

# Use C++17 everywhere, and fail if the compiler can't do it
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Program 1: the proxy. List every .cpp (not .hpp) it is built from.
add_executable(proxy
    src/main.cpp
    src/request-handler/client_handler.cpp
    src/request-handler/net_io.cpp
    src/socket/socket.cpp
    src/http/http_parser.cpp
)
target_include_directories(proxy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(WIN32)
    target_link_libraries(proxy PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(proxy PRIVATE Threads::Threads)
endif()

# Program 2: the parser tests. Separate program, so it has its own main.
# It compiles http_parser.cpp directly and needs no Winsock.
enable_testing()
add_executable(parser_test
    tests/http_parser_test.cpp
    src/http/http_parser.cpp
)
target_include_directories(parser_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME parser_test COMMAND parser_test)

# Warnings for both programs
if(MSVC)
    set(PROJECT_WARNING_FLAGS /W4)
else()
    set(PROJECT_WARNING_FLAGS -Wall -Wextra)
endif()
target_compile_options(proxy PRIVATE ${PROJECT_WARNING_FLAGS})
target_compile_options(parser_test PRIVATE ${PROJECT_WARNING_FLAGS})
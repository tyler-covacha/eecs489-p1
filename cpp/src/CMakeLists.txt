cmake_minimum_required(VERSION 3.10)

add_executable(iPerfer
    main.cpp
    client.cpp
    server.cpp
)

find_package(spdlog)
find_package(cxxopts)
# target_link_libraries(iPerfer PRIVATE cxxopts)

# Add your setup code above this line, then uncomment the line below

target_link_libraries(iPerfer spdlog::spdlog cxxopts::cxxopts)
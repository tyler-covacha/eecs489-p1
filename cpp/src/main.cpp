#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client.h"
#include "server.h"
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    int opt;
    cxxopts::Options options("iPerfer", "A simple network performance measurement tool");
    options.add_options()
        ("s, server", "Enable server", cxxopts::value<bool>())
        ("c, client", "Enable client", cxxopts::value<bool>())
        ("h, host", "Server Hostname/IP of iPerfer server", cxxopts::value<std::string>())
        ("p, port", "Port number to use", cxxopts::value<int>())
        ("t, time", "Duration in seconds for which data should be generated. int or float", cxxopts::value<float>()->default_value("10"));

    auto result = options.parse(argc, argv);
    bool is_server = false;
    bool is_client = false;
    std::string host = "";
    int port = -1;
    float time = 0;

    /* Server or Client */
    if (result.count("server") == 1) { 
        is_server = result["server"].as<bool>();
    }
    else if (result.count("client") == 1) {
        is_client = result["client"].as<bool>();
    }
    else {
        spdlog::error("Error: you must specify either only one server (-s) or client (-c) mode");
        exit(1);
    }

    /* Port */
    if (result.count("port") == 1) {
        port = result["port"].as<int>();
    }
    else if (result.count("port") != 1) {
        spdlog::error("Error: you must specify a port number with -p");
        exit(1);
    }
    spdlog::debug("About to check port number...");
    if (port < 1024 || port > 0xFFFF) {
      spdlog::error("Port number is: {}", port);
      spdlog::error("Error: port number must be in the range of [1024, 65535]"); 
      exit(1); 
    }

    /* Host */
    if (is_client) {
        if (result.count("host") == 1) {
            host = result["host"].as<std::string>();
        }
        else if (result.count("host") != 1) {
            spdlog::error("You must specify a hostname or IP address with -h when in client mode or too many hosts");
            exit(1);
        }

        if (result.count("time") == 1) {
            time = result["time"].as<float>();
            if (time <= 0) {
                spdlog::error("Error: time argument must be greater than 0");
                exit(1);
            }
        }
    }
    // spdlog::info(is_server ? "Running in server mode" : "Running in client mode");
    if (is_server) {
        runServer(port);
    }
    else if (is_client) {
        runClient(host, port, time);
    }

    return 0;
}
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "server.h"
#include <spdlog/spdlog.h>

// #define PORT 8080
#define MAX_LIST_SIZE 65536

void runServer(int PORT) {
    // Make a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)  {
        perror("error making socket");
        exit(1);
    }

    // Option for allowing you to reuse socket
    int yes {1};
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("error setsockopt");
        exit(1);
    }

    // Bind socket to a port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(sockfd, (sockaddr*)&addr, (socklen_t) sizeof(addr)) == -1) {
        perror("error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, 10) == -1) {
        perror("error listening");
        exit(1);
    }

    spdlog::info("iPerfer server started");



    while (true) {
        // Accept new connection
        struct sockaddr_in connection;
        socklen_t size = sizeof(connection);
        int connectionfd = accept(sockfd, (struct sockaddr*)&connection, &size);
        spdlog::info("Client connected");

        // Print what IP address connection is from
        char s[INET6_ADDRSTRLEN];
        printf("connetion from %s\n", inet_ntoa(connection.sin_addr));
        
        // Variable setup
        std::chrono::high_resolution_clock::time_point start, end;
        std::chrono::milliseconds rtt[7]; 
        int64_t rtt_last_four_sum = 0;

        /* Eight 'M' and 'A' messages */
        for (int i = 0; i < 8; i++) {
            // Receive message
            char buf[1024];
            int ret {};
            if ((ret = recv(connectionfd, buf, sizeof(buf), 0)) == -1) {
                perror("recv");
                close(connectionfd);
                continue;
            }
            buf[ret] = '\0';

            end = std::chrono::high_resolution_clock::now();
            if (i >= 1) {
                rtt[i-1] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                int64_t rtt_num = rtt[i-1].count();
                spdlog::info("rtt: {}", rtt_num);
                if (i >= 4) {
                    rtt_last_four_sum += rtt_num;
                }
                // printf("RTT %d: %ld milliseconds\n", i, rtt_num);
            }
            start = std::chrono::high_resolution_clock::now();
            printf("message: %s\n", buf);

            //Send ACK message back to client
            char ack[] = "A";
            if (send(connectionfd, ack, sizeof(ack), 0) == -1) {
                perror("send");
                close(connectionfd);
                continue;
            }
        }

        // Store Average RTT
        int average_rtt = rtt_last_four_sum / 4;

        // 80 KB chunks received and ACK sent

        int KB_received = 0;
        int counter = 0;
        while (true) {
            char buf[81920];
            int ret {};
            if ((ret = recv(connectionfd, buf, sizeof(buf), 0)) == -1) {
                perror("recv");
                close(connectionfd);
                break;
            }
            if (ret == 0) { // Connection closed by client
                break;
            }
            KB_received += ret / 1024;
            buf[ret] = '\0';

            // Send ACK message back to client
            char ack[] = "A";
            if (send(connectionfd, ack, sizeof(ack), 0) == -1) {
                perror("send");
                close(connectionfd);
                break;
            }
        }
        //Bandwidth calculation
        end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        float total_time_sec = (total_time.count() - average_rtt) / 1000.0;
        int Kb_received = KB_received * 8;
        float bandwidth = Kb_received / total_time_sec;

         spdlog::info("Received={} KB, Rate={:.3f} Mbps, Average RTT:{} ms\n", KB_received, bandwidth, average_rtt);

        close(connectionfd);
    }

    close(sockfd);
}
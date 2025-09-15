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
        // perror("error making socket");
        exit(1);
    }

    // Option for allowing you to reuse socket
    int yes {1};
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        // perror("error setsockopt");
        exit(1);
    }

    // Bind socket to a port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(sockfd, (sockaddr*)&addr, (socklen_t) sizeof(addr)) == -1) {
        // perror("error binding socket");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, 10) == -1) {
        // perror("error listening");
        exit(1);
    }

    spdlog::info("iPerfer server started");



    //while (true) {
        // Accept new connection
        struct sockaddr_in connection;
        socklen_t size = sizeof(connection);
        int connectionfd = accept(sockfd, (struct sockaddr*)&connection, &size);
        spdlog::info("Client connected");

        // Print what IP address connection is from
        // char s[INET6_ADDRSTRLEN];
        // printf("connetion from %s\n", inet_ntoa(connection.sin_addr));
        
        // Variable setup
        std::chrono::high_resolution_clock::time_point start, end;
        // std::chrono::milliseconds rtt[7]; 
        float rtt_last_four_sum = 0;

        /* Eight 'M' and 'A' messages */
        for (int i = 0; i < 8; i++) {
            // Receive message
            char buf[1];
            int ret {};
            if ((ret = recv(connectionfd, buf, sizeof(buf), 0)) == -1) {
                // perror("recv");
                close(connectionfd);
                continue;
            }
            end = std::chrono::high_resolution_clock::now();

            if (i >= 1) {
                // spdlog::info("Start {}, End {}", std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count(), std::chrono::duration_cast<std::chrono::milliseconds>(end.time_since_epoch()).count());
                // rtt[i-1] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                // float rtt_num = rtt[i-1].count();
                // spdlog::info("rtt: {}", rtt_num);

                float rtt_num = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
                
                if (i >= 4) {
                    rtt_last_four_sum += rtt_num;
                }
                // printf("RTT %d: %ld milliseconds\n", i, rtt_num);
            }
            start = std::chrono::high_resolution_clock::now();
            // printf("message: %s\n", buf);

            //Send ACK message back to client
            char ack[] = "A";
            if (send(connectionfd, ack, 1, 0) == -1) {
                // perror("send");
                close(connectionfd);
                continue;
            }
        }

        // Store Average RTT
        int average_rtt = rtt_last_four_sum / 4;

        // 80 KB chunks received and ACK sent

        int KB_received = 0;
        int counter = 0;
        int messages_sent = 0;
        const int expected = 81920;
        bool connection_closed = false;
        while (true) {
            int total_received = 0;
            char buf[expected];
            int ret {};
            if ((ret = recv(connectionfd, buf, expected, MSG_WAITALL)) == -1) {
                // perror("recv");
                close(connectionfd);
            };
            if (ret == 0) {
                break;
            }

            KB_received += ret / 1024;
            buf[ret] = '\0';

            // Send ACK message back to client
            char ack[] = "A";
            if (send(connectionfd, ack, 1, 0) == -1) {
                // perror("send");
                close(connectionfd);
                break;
            }
            messages_sent++;
        }
        //Bandwidth calculation
        end = std::chrono::high_resolution_clock::now();
        auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        float transmission_delay = (total_time.count() - (average_rtt * messages_sent));
        int Mb_received = KB_received / 125; 
        float bandwidth = (Mb_received) / (transmission_delay / 1000); //Mbpms CHANGE to Mbps

         spdlog::info("Received={} KB, Rate={:.3f} Mbps, RTT={}ms\n", KB_received, bandwidth, average_rtt);

        close(connectionfd);
        close(sockfd);
    //}
}
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client.h"
#include <spdlog/spdlog.h>

// #define PORT 8080

#define MICROSECONDS_IN_SECOND 1000000

void runClient(std::string hostName, int PORT, float time) {
    // Make a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)  {
        perror("error making socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    struct hostent *host = gethostbyname(hostName.c_str()); //"127.0.0.1"
    if (host == NULL) {
        perror("error gethostbyname");
        exit(1);
    }
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);
    addr.sin_port = htons(PORT);  // server port

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("error connecting");
        exit(1);
    }

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    float rtt_last_four_sum = 0;

    // Send 8 messages
    for (int i = 0; i < 8; i++) {
        char message[] = "M";
        start = std::chrono::high_resolution_clock::now();
        if (send(sockfd, message, 1, 0) == -1) {
            perror("send");
            exit(1);
        }

        char buf[1024];
        int ret {};
        if ((ret = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(1);
        }
        end = std::chrono::high_resolution_clock::now();

        // rtt
        if (ret >= 1) {
            float rtt_num = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start)).count();
                
            if (i >= 4) {
                rtt_last_four_sum += rtt_num;
            }
        }
    }

    int average_rtt = rtt_last_four_sum / 4;

    // Send messages for "time" seconds
    start = std::chrono::high_resolution_clock::now();
    end = std::chrono::high_resolution_clock::now();
    auto startTime = (std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch())).count();
    auto endTime = (std::chrono::duration_cast<std::chrono::microseconds>(end.time_since_epoch())).count();
    float timeElapsed = endTime - startTime;
    int KB_sent = 0;
    int messages_sent = 0;

    char message[81920] = {'\0'};
    while ((timeElapsed / MICROSECONDS_IN_SECOND) < time){
        if (send(sockfd, message, sizeof(message), 0) == -1) {
            perror("send");
            exit(1);
        }
        KB_sent += 80; // 80 KB
        char buf[1024];
        messages_sent++;
        int ret {};
        if ((ret = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(1);
        }
        end = std::chrono::high_resolution_clock::now();
        endTime = (std::chrono::duration_cast<std::chrono::microseconds>(end.time_since_epoch())).count();
        timeElapsed = endTime - startTime;
        spdlog::info("Time Elapsed: {:.3f} seconds", (timeElapsed / MICROSECONDS_IN_SECOND));
    }

    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    float transmission_delay = (total_time.count() - (average_rtt * messages_sent)) / 1000.0;
    int Kb_sent = KB_sent * 8;
    float bandwidth = Kb_sent / transmission_delay;

    spdlog::info("Sent={} KB, Rate={:.3f} Mbps, Average RTT:{} ms\n", KB_sent, bandwidth, average_rtt);



    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
}
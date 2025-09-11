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

    // Send a message
    for (int i = 0; i < 8; i++) {
        char message[] = "M";
        if (send(sockfd, message, sizeof(message), 0) == -1) {
            perror("send");
            exit(1);
        }

        char buf[1024];
        int ret {};
        if ((ret = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[ret] = '\0';

    }

    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);
}
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1" // Change this to the server's IP address
#define SERVER_PORT 53030
#define MAX_PACKET_SIZE 8192 // Maximum packet size
#define FILENAME "received_image.jpg" // Name for the received image file
#define TIMEOUT_SECONDS 5

int performRequest(const char request[], int client_socket) {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);

     // Replace this with your string to send

    // Send the string to the server
    if (sendto(sockfd, request, strlen(request), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(sockfd);
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return -1;
    }

    FILE *fp = fopen(FILENAME, "wb"); // Open file for writing in binary mode
    if (fp == NULL) {
        perror("File open failed");
        close(sockfd);
        return -1;
    }

    int bytes_received;
    char buffer[MAX_PACKET_SIZE];

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    int select_result = select(sockfd + 1, &fds, NULL, NULL, &timeout);
    if (select_result == -1) {
        perror("select");
        fclose(fp);
        close(sockfd);
        return -1;
    } else if (select_result == 0) {
        printf("408 Request Timeout: Server not responding within %d seconds.\n", TIMEOUT_SECONDS);
        fclose(fp);
        close(sockfd);
        return -1;
    } else {
        while (1) {
            if ((bytes_received = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0, NULL, NULL)) == -1) {
                perror("recvfrom");
                fclose(fp);
                close(sockfd);
                return -1;
            }

            if (strncmp(buffer, "DONE", 4) == 0) {
                printf("Received DONE signal from server. File transfer complete.\n");
                break;
            }

            if (strncmp(buffer, "File Not Found", 14) == 0) {
                printf("FILE NOT FOUND\n");
                break;
            }

            // Write received data to the file
            fwrite(buffer, 1, bytes_received, fp);
        }
    }

    fclose(fp); // Close the file
    close(sockfd);

    return 0;
}

int main() {
    char request[] = "fat cat";
    if (performRequest(request, 0) != 0) {
        printf("Request failed.\n");
    }

    return 0;
}

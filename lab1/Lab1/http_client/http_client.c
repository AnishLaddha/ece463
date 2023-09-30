/* The code is subject to Purdue University copyright policies.
 * DO NOT SHARE, DISTRIBUTE, OR POST ONLINE
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

char* extractFileName(char* path);
void extractHeader(char* response, char* header);

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "usage: ./http_client [host] [port number] [filepath]\n");
        exit(1);
    }
    char* host = argv[1];
    int port = atoi(argv[2]);
    char* filepath = argv[3];
    char* filename = extractFileName(filepath);


    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket() failed\n");
        exit(1);
    }

    // Get host
    struct hostent* server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "gethostbyname() failed\n");
        exit(1);
    }

    //server address
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr)); //clears serveraddr
    serveraddr.sin_family = AF_INET; //sets family to AF_INET
    serveraddr.sin_addr = *((struct in_addr *)server->h_addr_list[0]); //sets address from list
    serveraddr.sin_port = htons(port); //sets port

    // Connect to server
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        fprintf(stderr, "connect() failed\n");
        exit(1);
    }

    // construct HTTP request
    char request[1024];
    snprintf(request, 1024, "GET %s HTTP/1.0\r\nHost: %s:%d\r\n\r\n", filepath, host, port);
    //send request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        fprintf(stderr, "send() failed\n");
        exit(1);
    }


    //open file
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //recv file
    char response[1024];
    char header[1024];
    int bytes_received=0;
    int content_len = -1;
    int num_bytes;
    int flag = 0;
    while(1)
    {
        num_bytes = recv(sockfd, response, 1024-1, 0);
        if (num_bytes < 0) {
            fprintf(stderr, "recv() failed\n");
            exit(1);
        }
        else if (num_bytes == 0) {
            break;
        }
        response[num_bytes] = '\0';
        // if (flag == 0){
        //     printf("%s", response);
        // }
        
        if(content_len == -1)
        {
            char* content_len_loc = strstr(response, "Content-Length: ");
            if(content_len_loc != NULL)
            {
                content_len_loc += strlen("Content-Length: ");
                content_len = atoi(content_len_loc);
            }
            extractHeader(response, header);
            printf("%s", header);
            //flag = 1;
        }
        fwrite(response, sizeof(char), num_bytes, file);

        bytes_received += num_bytes;
        if (content_len != -1 && bytes_received >= content_len && ftell(file) >= content_len) {
            break;
        }

    }
    fclose(file);
    return 0;
}



char* extractFileName(char* path) {
    char* lastSlash = strrchr(path, '/');
    
    if (lastSlash != NULL) {
        // Return the substring after the last '/'
        return lastSlash + 1;
    }
    
    // If no '/', return the original path
    return path;
}

void extractHeader(char* response, char* header) {
    char* occurrence = strstr(response, "\r\n\r\n");
    
    if (occurrence != NULL) {
        // Calculate the length of the header
        size_t headerLength = occurrence - response;
        
        // Copy the content to the header buffer
        strncpy(header, response, headerLength);
        header[headerLength] = '\0';  // Null-terminate the string
        
        // Update the response to contain everything after the occurrence
        strcpy(response, occurrence + 4);  // Skip the "\r\n\r\n"
    } else {
        // If no "\r\n\r\n" found, header is the same as the original response
        strcpy(header, response);
        response[0] = '\0';  // Clear the original response
    }
}

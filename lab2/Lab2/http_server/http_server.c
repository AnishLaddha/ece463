#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 8030
#define BUFFER_SIZE 4096
#define DBADDR "127.0.0.1" // Change this to the server's IP address
#define DBPORT 53030
#define MAX_PACKET_SIZE 4096 // Maximum packet size
#define TIMEOUT_SECONDS 5


void printSpecialChars(const char arr[], int n) {
    for (int i = 0; (i<n & arr[i] != '\0'); i++) {
        switch (arr[i]) {
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\0':
                printf("\\0");
                break;
            default:
                printf("%c", arr[i]);
                break;
        }
    }
    //printf(" ");
}
void print_ip(const char arr[], int n) {
    for (int i = 0; (i<n & arr[i] != '\0'); i++) {
        switch (arr[i]) {
            case '\n':
                printf("\\n");
                break;
            case '\r':
                printf("\\r");
                break;
            case '\0':
                printf("\\0");
                break;
            default:
                printf("%c", arr[i]);
                break;
        }
    }
    printf(" ");
}


char* db_extract_str(const char* input, int n) {
    // Allocate memory for the output string
    char* output = (char*)malloc(n * sizeof(char));
    if (output == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    int outputIndex = 0;
    int inputLength = strlen(input);

    // Look for the substring "?key="
    const char* keyString = "?key=";
    const char* keyPosition = strstr(input, keyString);

    if (keyPosition != NULL) {
        keyPosition += strlen(keyString); // Move to the beginning of the value

        // Copy characters to the output until encountering a '&' or end of string
        while (*keyPosition != '\0' && *keyPosition != '&') {
            // Replace '+' with space
            if (*keyPosition == '+') {
                output[outputIndex++] = ' ';
            } else {
                output[outputIndex++] = *keyPosition;
            }
            keyPosition++;
        }

        // Null-terminate the output string
        output[outputIndex] = '\0';
    } else {
        free(output);
        return NULL;
    }

    return output;
}


int performRequest(const char request[], int client_socket) {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DBPORT);
    server_addr.sin_addr.s_addr = inet_addr(DBADDR);
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


    int bytes_received;
    char buffer[MAX_PACKET_SIZE];
    int flag_404=0;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    int select_result = select(sockfd + 1, &fds, NULL, NULL, &timeout);
    if (select_result == -1) {
        perror("select");
        close(sockfd);
        return -1;
    } 
    else if (select_result == 0) {
        //printf("408 Request Timeout: Server not responding within %d seconds.\n", TIMEOUT_SECONDS);
        char response[] = "HTTP/1.0 408 Request Timeout\r\n\r\n<html><body><h1>408 Request Timeout</h1></body></html>";
        write(client_socket, response, strlen(response));
        close(sockfd);
        return 3;
    } 
    else {
        char response_header[] = "HTTP/1.0 200 OK\r\n\r\n";
        write(client_socket, response_header, strlen(response_header));
        while (1) {
            if ((bytes_received = recvfrom(sockfd, buffer, MAX_PACKET_SIZE, 0, NULL, NULL)) == -1) {
                perror("recvfrom");
                close(sockfd);
                return -1;
            }

            if (strncmp(buffer, "DONE", 4) == 0) {
                //printf("Received DONE signal from server. File transfer complete.\n");
                break;
            }

            if (strncmp(buffer, "File Not Found", 14) == 0) {
                //printf("FILE NOT FOUND\n");
                char response[] = "HTTP/1.0 404 Not Found\r\n\r\n <html><body><h1>404 Not Found</h1></body></html>";
                write(client_socket, response, strlen(response));
                flag_404 = 1;
                break;
            }
            else
            {
                write(client_socket, buffer, bytes_received);
            }            
        }
    }

    close(sockfd);
    if(flag_404 == 1)
    {
        return 1;
    }

    return 2;
}



char* filePathGen(int length, char* filename)
{
    char* fpgen;
    if(filename[length-1] == '/')
    {
        fpgen = (char*)malloc((17+length)*sizeof(char));
        strcpy(fpgen, "Webpage");
        strcat(fpgen, filename);
        strcat(fpgen, "index.html");

        fpgen[17+length] = '\0';
    }
    else
    {
        fpgen = (char*)malloc((7+length)*sizeof(char));
        strcpy(fpgen, "Webpage");
        strcat(fpgen, filename);
        fpgen[7+length] = '\0';
    }

    return fpgen;
}



int handle_client(int client_socket) {
    int status=0;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Read the request from the client
    read(client_socket, buffer, BUFFER_SIZE - 1);
    //printf("Received request:\n %s", buffer);

    // Check if the request is a GET method
    if (strstr(buffer, "GET") == NULL) 
    {
        // If it's not a GET method, send a 501 response
        char response[] = "HTTP/1.1 501 Not Implemented\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
        write(client_socket, response, strlen(response));
    } 
    else 
    {

        char *newline = strchr(buffer, '\n');
        char response[newline - buffer + 1];
        strncpy(response, buffer, newline - buffer);
        response[newline - buffer] = '\0';
        printf("\"");
        printSpecialChars(response, newline-buffer - 1);
        printf("\" ");
        // Extracting the filepath between "GET" and "HTTP"
        char *getPos = strstr(buffer, "GET ");
        char *key;
        char* filepath;
        if(getPos != NULL) {
            getPos += 4; // Move to the character after "GET "
            char *httpPos = strstr(getPos, " HTTP");
            if (httpPos != NULL) {
                size_t length = httpPos - getPos;
                char fname[length + 1];
                strncpy(fname, getPos, length);
                fname[length] = '\0';
                key = db_extract_str(fname, length+1);
                filepath = filePathGen((int)length, fname);

                //printf( response);
                
            }
        }

        if(key == NULL)
        {
            //printf("FILEPATH: %s\n\n\n", filepath);
            FILE *file = fopen(filepath, "rb");
            if (file == NULL) 
            {
                char response[] = "HTTP/1.1 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
                write(client_socket, response, strlen(response));
                status = 1;
            } 
            else 
            {
                fseek(file, 0L, SEEK_END);
                int sz = ftell(file);
                fseek(file, 0L, SEEK_SET);

                char response_header[60]; //= "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
                sprintf(response_header, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n\r\n", sz);
                //printf(response_header);
                write(client_socket, response_header, strlen(response_header));

                // Read and send the content of index.html
                int size;
                while ((size = fread(buffer, 1, BUFFER_SIZE, file)) != 0) 
                {
                    write(client_socket, buffer, size);
                }
                fclose(file);
                status = 2;
            }
        }
        else
        {
            status = performRequest(key, client_socket);
        }
    }

        

    if(status == 0)
    {
        printf("%s\n", "501 Not Implemented");
    }
    else if(status == 1)
    {
        printf("%s\n", "404 Not Found");
    }
    else if(status == 2)
    {
        printf("%s\n", "200 OK");
    }
    else if(status == 3)
    {
        printf("%s\n", "408 Request Timeout");
    }

    // Close the client socket
    close(client_socket);

    return status;
}




int main() 
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address settings
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    //printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        int st = -1;
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Acceptance failed");
            exit(EXIT_FAILURE);
        }

        // Handle the client's request
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        //printf
        //printf("%s \n", client_ip);
        print_ip(client_ip, INET_ADDRSTRLEN);
        st = handle_client(client_socket);

    }



    // Close the server socket
    close(server_socket);

    return 0;
}
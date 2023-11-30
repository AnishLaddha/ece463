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
#define BUFFER_SIZE 1024

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
    printf(" ");
}

char* db_extract_str(char* input) {
    char* key = "key=";
    char* extracted = strstr(input, key);

    if (extracted != NULL) 
    {
        extracted += strlen(key);

        // Replace '+' with ' '
        for (int i = 0; extracted[i] != '\0'; ++i) 
        {
            if (extracted[i] == '+') 
            {
                extracted[i] = ' ';
            }
        }

        return extracted;
    } 
    else 
    {
        return NULL;
    }
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
        //printf("%s", response);
        printSpecialChars(response, newline-buffer - 1);
        // Extracting the filepath between "GET" and "HTTP"
        char *getPos = strstr(buffer, "GET ");

        char* filepath;
        if(getPos != NULL) {
            getPos += 4; // Move to the character after "GET "
            char *httpPos = strstr(getPos, " HTTP");
            if (httpPos != NULL) {
                size_t length = httpPos - getPos;
                char fname[length + 1];
                strncpy(fname, getPos, length);
                fname[length] = '\0';

                filepath = filePathGen((int)length, fname);

                //printf( response);
                //printf("Filename: %s\n", fname);
            }
        }

        
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
    if(status == 0)
    {
        printf("%s\n", "501 Not Implemented");
    }
    else if(status == 1)
    {
        printf("%s\n","404 Not Found");
    }
    else if(status == 2)
    {
        printf("%s\n","200 OK");
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
        client_ip[INET_ADDRSTRLEN] = '\0';
        //printf
        //printf("%s \n", client_ip);
        printSpecialChars(client_ip, INET_ADDRSTRLEN);
        st = handle_client(client_socket);

    }



    // Close the server socket
    close(server_socket);

    return 0;
}
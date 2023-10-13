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
//void true_print(char* arr);


int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "usage: ./http_client [host] [port number] [filepath]\n");
        exit(1);
    }
    char *host = argv[1];
    int port = atoi(argv[2]);
    char *file_path = argv[3];
    char *file_name = extractFileName(file_path);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
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
    if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        fprintf(stderr, "connect() failed\n");
        exit(1);
    }


  // Send HTTP GET request
    char request[1024];
    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", file_path, host);
    if (send(sock, request, strlen(request), 0) < 0) {
      perror("send");
      close(sock);
      exit(1);
    }

  // Receive HTTP response
    char buffer[4096];
    char headers[4096];
    char buf2[1024];
    FILE *fp;

    int n;
    int header_done = 0;
    int header_almost_done = 0;
    int prog_out = 0;
    long conlen = -1;
    long bytes_recv = 0;
    int status_code;
    while(1) 
    {
        memset(buffer, 0, sizeof(buffer));
        n = recv(sock,buffer, sizeof(buffer), 0);
        if(n<0)
        {
            break;
        }
        if(!header_done)
        {
            //printf("%s\n", buffer);
            if(prog_out == 0)
            {
                char *httpLineStart = strstr(buffer, "HTTP");
                strncpy(buf2, buffer, 1024);
                char *line_1 = strtok(buf2, "\n");
                if(line_1[strlen(line_1)-1] == '\r')
                {
                    line_1[strlen(line_1)-1] = '\0';
                }
                if(sscanf(line_1, "HTTP/%*f %d", &status_code) != 1)
                {
                    exit(1);
                }
                if(status_code != 200)
                {
                    printf("%s\n", line_1);
                    exit(1);
                }
                prog_out = 1;
            }


            char* content_length = strstr(buffer, "Content-Length: ");
            if(content_length != NULL)
            {
                content_length+=strlen("Content-Length: ");
                conlen = atoi(content_length);
                //printf("Content-Length: %ld\n", conlen);

            }
            else{
                printf("Error: could not download the requested file (file length unknown)\n");
                exit(1);
            }


            char* end_of_header = strstr(buffer, "\r\n\r\n");
            if(end_of_header != NULL)
            {
                end_of_header+=4;
                //printf("%s\n", end_of_header);
                int body_len = n - (end_of_header - buffer);
                fp = fopen(file_name, "wb");
                if (!fp) {
                    perror("fopen");
                    close(sock);
                    exit(1);
                }
                fwrite(end_of_header, 1, body_len, fp);
                bytes_recv += body_len;
                header_almost_done = 1;
            }
        }
        
        if(n>0)
        {
            if(!header_almost_done)
            {
                fwrite(buffer, 1, n, fp);
                bytes_recv += n;
            }
            else
            {
                header_done = 1;
                header_almost_done = 0;
            }
            // fwrite(buffer, 1, n, fp);
            // bytes_recv += n;
        } 
        if(bytes_recv >= conlen)
        {
            break;
        }
    }
    fclose(fp);
    close(sock);
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
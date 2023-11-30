/* The code is subject to Purdue University copyright policies.
 * DO NOT SHARE, DISTRIBUTE, OR POST ONLINE
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>

#define MYPORT 8030
#define BACKLOG 10

int main() {


	int sockfd, new_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr; /* client's address info */
	int sin_size;
	char dst[INET_ADDRSTRLEN];


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

    //option 1: to create a non-blocking socket
    //int status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

    //option 2: to create a non-blocking socket (only works on Linux > 2.6.27)
	//if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
	//	perror("socket");
	//	exit(1);
	//}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY; /* bind to all local interfaces */
	bzero(&(my_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) < 0) {
		perror("listen");
		exit(1);
	}

	while(1) {
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd,
        (struct sockaddr *) &their_addr, &sin_size)) < 0) {
			perror("accept");
			continue;
		}

		inet_ntop(AF_INET, &(their_addr.sin_addr), dst, INET_ADDRSTRLEN);
		printf("server: got connection from %s\n", dst);

        char str[1000];
        FILE* inp = fopen("img.txt", "r");
        printf("Sending the file...\n");
        if (inp) {
            while (fgets(str, 1000, inp)) {
                //printf("%s",str);
                if(send(new_fd, str, 1000, 0) < 0) {
                    perror("send");
                    exit(1);
                }
                usleep(100000);
            }
            strcpy(str, "DONE");
            if(send(new_fd, str, 1000, 0) < 0) {
                perror("send");
                exit(1);
            }
            fclose(inp);

        } else {
            perror("read");
            exit(1);
        }

        printf("Send successfully completed...\n");

		close(new_fd);
	}

	close(sockfd);

    return 0;
}


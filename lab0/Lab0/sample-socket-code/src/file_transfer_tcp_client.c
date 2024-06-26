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

#define SERVPORT 8030

int main(int argc, char *argv[]) {
	int sockfd, numbytes;
	struct sockaddr_in their_addr; /* client's address information */
	struct hostent* he;
    char str[1000];

	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

    /* get server's IP by invoking the DNS */
	if ((he = gethostbyname(argv[1])) == NULL) {
		herror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(SERVPORT);
	their_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);
	bzero(&(their_addr.sin_zero), 8);

	if (connect(sockfd, (struct sockaddr *) &their_addr,
    sizeof(struct sockaddr)) < 0) {
		perror("connect");
		exit(1);
	}

    while (1) {
        if ((numbytes = recv(sockfd, str, 1000, 0)) < 0) {
            perror("recv");
            exit(1);
        }

        if (strstr(str, "DONE")!=NULL) break;
        printf("%s", str);
    }

	close(sockfd);

	return 0;
}


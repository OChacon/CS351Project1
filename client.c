/*
 * Title: client.c
 * Author: Oscar Chacon (orc2815@rit.edu)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define MYPORT 3490
#define DEST_IP "10.12.110.57"
#define DEST_PORT 1300

int main(int argc, char const *argv[]) {
    struct sockaddr_in addr;
    int sockfd;
    char buffer[1024];
    //struct hostnet *server;
    char *msg = "This is the client";
/*
    if(argc != 2){
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    if((server = gethostbyname(argv[1])) == NULL){
        perror("gethostbyname");
        exit(1);
    }
*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(addr.sin_zero), '\0', 8);

    int c = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));

    if(c == -1){
        perror("recv");
        exit(1);
    }

    long s = send(sockfd, msg, strlen(msg), 0);
    if(s == -1){
        perror("didn't send");
        exit(1);
    }
    printf("Message Sent\n");

    recv(sockfd, buffer, 1024, 0);

    printf("Ping back");
    close(sockfd);

    return 0;
}


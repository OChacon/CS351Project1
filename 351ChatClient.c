/*
 * Title: 351ChatClient.c
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

int main(int argc, char const *argv[]) {
    struct sockaddr_in addr;
    int sockfd;
    char buffer[1024];
    char *r_msg;
    char *s_msg;
    char *credentials;
    const char *usrname;
    const char *psswrd;
    u_short DEST_IP;
    u_short DEST_PORT;

    //First check if # of args are correct
    if(argc != 4){
        fprintf(stderr, "Invalid arguments: must provide an IP, port number, username, and password\n");
        exit(1);
    }

    //Check whether they say localhost or 127.0.0.1 for IP
    if(strcmp(argv[0], "localhost") == 0){
        DEST_IP = (u_short) strtoul("127.0.0.1", NULL, 0);
    }
    else{
        DEST_IP = (u_short) strtoul(argv[0], NULL, 0);
    }

    DEST_PORT = (u_short) strtoul(argv[1], NULL, 0);
    usrname = argv[2];
    psswrd = argv[3];

    //Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("Unable to create socket\n");
        exit(1);
    }

    //Create the address
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEST_PORT);
    addr.sin_addr.s_addr = DEST_IP;
    memset(&(addr.sin_zero), '\0', 8);

    //Try and connect
    int c = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if(c == -1){
        perror("Cannot connect to server\n");
        exit(1);
    }

    //- - - - - NEEDS MORE WORK PAST HERE FOR LOOP - - - - -

    //Send the credentials of the user
    credentials = malloc(strlen(usrname) + 1 + strlen(psswrd));
    strcpy(credentials, usrname);
    strcat(credentials, " ");
    strcat(credentials, psswrd);

    r_msg = malloc(sizeof(buffer));
    s_msg = malloc(sizeof(buffer));

    strcpy(s_msg, credentials);

    while(strcmp(s_msg, "#EXIT") != 0) {

        printf("> ");
        gets(s_msg);

        long s2 = send(sockfd, s_msg, strlen(s_msg), 0);
        if (s2 == -1) {
            perror("Didn't send message\n");
            exit(1);
        }

        long r2 = recv(sockfd, buffer, 1024, 0);
        if (r2 == -1) {
            perror("Didn't receive anything from server\n");
            exit(1);
        }
        else{
            for(int i = 0; i < r2; i++){
                strcat(r_msg, &buffer[i]);
            }
            printf(r_msg);
        }
    }

    close(sockfd);
    return 0;
}


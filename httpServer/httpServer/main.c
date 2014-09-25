//
//  main.c
//  httpServer
//
//  Created by nosources on 14-9-25.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include "socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int main(int argc, const char * argv[])
{

    int listen_fd;
    int connect_fd;
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    if (!init_socket(&listen_fd, &server_addr)) {
        printf("error init socket.");
        exit(EXIT_FAILURE);
    }
    
    socklen_t addr_len = sizeof(struct sockaddr_in);
    pid_t pid;
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(struct sockaddr_in));
    while (1) {
        connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (-1 == connect_fd) {
            printf("accept call error");
            continue;
        }
        pid = fork();
        if (pid > 0) {
            close(connect_fd);
            continue;
        }else if(pid == 0){
            close(listen_fd);
            printf("pid is %d, http sesson from %s:%d", getpid(), inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
            shutdown(connect_fd, SHUT_RDWR);
            exit(EXIT_SUCCESS);
        }else{
            printf("fork call error");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}













































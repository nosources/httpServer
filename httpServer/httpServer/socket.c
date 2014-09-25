//
//  socket.c
//  httpServer
//
//  Created by nosources on 14-9-25.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#include <stdio.h>
#include "socket.h"
int init_socket(int *listen_fd, struct sockaddr_in *server_addr){
    *listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*listen_fd == -1) {
        printf("socket call error");
        return -1;
    }
    
    int re_use_addr = SO_REUSEADDR;
    if (-1 == setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &re_use_addr, sizeof(re_use_addr))) {
        printf("setsockopt call error");
        return -1;
    }
    
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(8080);
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (-1 == bind(*listen_fd, (struct sockaddr *)server_addr, sizeof(struct sockaddr_in))) {
        printf("bind call error");
        return -1;
    }
    
    if (-1 == listen(*listen_fd, 20)) {
        printf("listen call error");
        return -1;
    }
    return 0;
}
//
//  socket.h
//  httpServer
//
//  Created by nosources on 14-9-25.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#ifndef httpServer_socket_h
#define httpServer_socket_h

#include <netinet/in.h>
int init_socket(int *listen_fd, struct sockaddr_in *server_addr);

#endif

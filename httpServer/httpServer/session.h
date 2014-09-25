//
//  session.h
//  httpServer
//
//  Created by nosources on 14-9-25.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#ifndef httpServer_session_h
#define httpServer_session_h

#include <netinet/in.h>
int http_session(int *connect_fd, struct sockaddr_in *client_addr);

#endif

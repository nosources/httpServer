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

/* if http protocol return 1, else return 0 */
int is_http_protocol(char *msg_from_client);

/* get the request header's uri */
char *get_uri(char *req_header, char *uri_buf);

/* get the uri status,access return 0, not exist return 1, permission deny return 2, error return -1 */
int get_uri_status(char *uri);

/* get the mime type of the file request in uri from client's browse */
char *get_mime_type(char *uri);
#endif

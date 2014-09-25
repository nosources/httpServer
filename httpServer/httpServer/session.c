//
//  session.c
//  httpServer
//
//  Created by nosources on 14-9-25.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#include <stdio.h>
#include "session.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define RECV_BUFFER_SIZE	1024		/* 1KB of receive buffer  */
#define	SEND_BUFFER_SIZE	1050000		/* 1.xMB of send buffer  */
#define	URI_SIZE			128			/* length of uri request from client browse */

#define TIME_OUT_SEC		10			/* select timeout of secend */
#define TIME_OUT_USEC		0			/* select timeout of usecend */

#define	FILE_OK				200
#define	FILE_FORBIDEN		403			/* there are no access permission*/
#define FILE_NOT_FOUND		404			/* file not found on server */
#define	UNALLOW_METHOD		405			/* un allow http request method*/
#define FILE_TOO_LARGE		413			/* file is too large */
#define	URI_TOO_LONG		414			/*  */
#define	UNSUPPORT_MIME_TYPE	415
#define	UNSUPPORT_HTTP_VERSION	505
#define	FILE_MAX_SIZE		1048576		/* 1MB the max siee of file read from hard disk */


#define ALLOW				"Allow:GET"	/* the server allow GET request method*/
#define	SERVER				"Server:Mutu(0.1 Alpha)/Linux"
int http_session(int *connect_fd, struct sockaddr_in *client_addr){
    char recv_buf[RECV_BUFFER_SIZE + 1] = {0};
    unsigned char send_buf[SEND_BUFFER_SIZE +1] = {0};
    unsigned char file_buf[FILE_MAX_SIZE +1] = {0};
    
    char uri_buf[URI_SIZE +1] = {0};
    
    int maxfd = *connect_fd + 1;
    fd_set read_set;
    FD_ZERO(&read_set);
    
    struct timeval  timeout;
    timeout.tv_sec = TIME_OUT_SEC;
    timeout.tv_usec = TIME_OUT_USEC;
    
    int res = 0;
    int read_bytes = 0;
    int send_bytes = 0;
    int file_size = 0;
    char *mime_type;
    int uri_status;
    
    FD_SET(*connect_fd, &read_set);
    while (ture) {
        res = select(maxfd, &read_set, NULL, NULL, &timeout);
        switch (res) {
            case -1:
                printf("select call error");
                close(*connect_fd);
                return  -1;
            case 0:
                continue;
            default:
                if (FD_ISSET(*connect_fd, &read_set)) {
                    memset(recv_buf, '\0', sizeof(recv_buf));
                    read_bytes = recv(*connect_fd, recv_buf, RECV_BUFFER_SIZE, 0);
                    printf("receive data is %s", recv_buf);
                    if (read_bytes == 0) {
                        return 0;
                    }else if(read_bytes > 0){
                        if (0 == is_http_protocol(recv_buf)) {
                            printf("request is not http protocol");
                            close(*connect_fd);
                            return -1;
                        }else{
                            memset(uri_buf, '\0', sizeof(uri_buf));
                            if (NULL == get_uri(recv_buf, uri_buf)) {
                                uri_status = URI_TOO_LONG;
                            }else{
                                printf("uri is \"%s\"", uri_buf);
                                uri_status = get_uri_status(uri_buf);
                                switch (uri_status) {
                                    case FILE_OK:
                                        mime_type = get_mime_type(uri_buf);
                                        file_size = get_file_disk(uri_buf, file_buf);
                                        send_bytes = reply_normal_information(send_buf, file_buf, file_size, mime_type);
                                        break;
                                    case FILE_NOT_FOUND:
                                        send_bytes = send_error_information(send_buf, FILE_NOT_FOUND);
                                        break;
                                    case FILE_FORBIDEN:
                                        break;
                                    case URI_TOO_LONG:
                                        break;
                                    default:
                                        break;
                                }
                            }
                            send(*connect_fd, send_buf, send_bytes, 0);
                        }
                    }
                }
        }
    }
    return 0;
}

int is_http_protocol(char *msg_from_client){
    /*
     GET / HTTP/1.1
     Host: localhost:8080
     User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_3) AppleWebKit/534.53.11 (KHTML, like Gecko) Version/5.1.3 Safari/534.53.10
     Accept: text/html,application/xhtml+xml,application/xml;q=0.9
    Accept-Language: zh-cn
    Accept-Encoding: gzip, deflate
Connection: keep-alive
    */
    if (0 != strncmp(msg_from_client, "GET", 3)) {
        return 0;
    }
    if (0!= ) {
        <#statements#>
    }
}



















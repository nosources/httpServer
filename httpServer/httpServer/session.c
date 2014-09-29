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
#include <string.h>
#include <regex.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "hashmap.h"
#include <time.h>

struct hashmap hm;
void init_mime_type_hm(){
    hashmap_init(&hm, 0, hash_text, cmp_text, NULL, NULL);
    hashmap_put(&hm, "html", "text/html");
    hashmap_put(&hm, "htm", "text/html");
    hashmap_put(&hm, "jpeg", "image/jpeg");
    hashmap_put(&hm, "jpg", "image/jpeg");
    hashmap_put(&hm, "css", "text/css");
    hashmap_put(&hm, "png", "image/png");
    hashmap_put(&hm, "js", "text/javascript");
}

char *get_time_str(char *time_buf)
{
	time_t	now_sec;
	struct tm	*time_now;
	if(	time(&now_sec) == -1)
	{
		perror("time() in get_time.c");
		return NULL;
	}
	if((time_now = gmtime(&now_sec)) == NULL)
	{
		perror("localtime in get_time.c");
		return NULL;
	}
	char *str_ptr = NULL;
	if((str_ptr = asctime(time_now)) == NULL)
	{
		perror("asctime in get_time.c");
		return NULL;
	}
	strcat(time_buf, str_ptr);
	return time_buf;
}


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
    char *mime_type = NULL;
    int uri_status;
    
    FD_SET(*connect_fd, &read_set);
    while (1) {
        res = select(maxfd, &read_set, NULL, NULL, &timeout);
        switch (res) {
            case -1:
                printf("select call error");
                close(*connect_fd);
                return  -1;
                break;
            case 0:
                continue;
                break;
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
                                        send_bytes = set_error_information(send_buf, FILE_NOT_FOUND);
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
    return 1;
}

char *get_uri(char *req_header, char *uri_buf){
    regex_t exp;
    if (0 != regcomp(&exp, "GET /(.*) HTTP", REG_EXTENDED)) {
        printf("regcomp call error");
        return NULL;
    }
    regmatch_t match[2];
    if (REG_NOMATCH != regexec(&exp, req_header, 2, match, REG_NOTBOL)) {
        if (match[1].rm_eo - match[1].rm_so == 0) {
            strcpy(uri_buf, "index.html");
        }else{
            strncpy(uri_buf, req_header + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
        }
        
        return uri_buf;
    }
    regfree(&exp);
    return NULL;
    
}

int get_uri_status(char *uri){
    if (-1 == access(uri, F_OK)) {
        printf("file %s not found", uri);
        return FILE_NOT_FOUND;
    }
    if (-1 == access(uri, R_OK)) {
        printf("file %s found, can't read", uri);
        return FILE_FORBIDEN;
    }
    return FILE_OK;
}

char * get_mime_type(char *uri){
    regex_t exp;
    if (0 != regcomp(&exp, ".*\\.([a-zA-Z]+)", REG_EXTENDED)) {
        printf("regcomp call error");
        return NULL;
    }
    regmatch_t match[2];
    char key[10] = {0};
    if (REG_NOMATCH != regexec(&exp, uri, 2, match, REG_NOTBOL)) {
        strncpy(key, uri + match[1].rm_so, match[1].rm_eo - match[1].rm_so);
    }
    return hashmap_get(&hm, key);
}

int get_file_disk(char *uri, unsigned char *file_buf){
    int read_count = 0;
    int fd = open(uri, O_RDONLY);
    if (-1 == fd) {
        printf("open %s fail", uri);
        return -1;
    }
    
    unsigned long st_size;
    struct stat st;
    if (-1 == fstat(fd, &st)) {
        printf("fstat %s failed", uri);
        return -1;
    }
    st_size = st.st_size;
    if (st_size > FILE_MAX_SIZE) {
        printf("file %s is large than FILE_MAX_SIZE", uri);
        return -1;
    }
    read_count = read(fd, file_buf, FILE_MAX_SIZE);
    if (-1 == read_count) {
        printf("read %s failed", uri);
        return -1;
    }
    printf("file %s size: %lu, read: %d", uri, st_size, read_count);
    return read_count;
}

int set_error_information(unsigned char *send_buf, int errorno){
    register int index = 0;
    register int len = 0;
    char *str = NULL;
    switch (errorno) {
        case FILE_NOT_FOUND:
            printf("In set_error_information FILE_NOT_FOUND case\n");
			str = "HTTP/1.1 404 File Not Found\r\n";
			len = strlen(str);
			memcpy(send_buf + index, str, len);
			index += len;
            
			len = strlen(SERVER);
			memcpy(send_buf + index, SERVER, len);
			index += len;
            
			memcpy(send_buf + index, "\r\nDate:", 7);			
			index += 7;
			
			char time_buf[TIME_BUFFER_SIZE];
			memset(time_buf, '\0', sizeof(time_buf));
			get_time_str(time_buf);
			len = strlen(time_buf);
			memcpy(send_buf + index, time_buf, len);
			index += len;
            
			str = "\r\nContent-Type:text/html\r\nContent-Length:";
			len = strlen(str);
			memcpy(send_buf + index, str, len);
			index += len;
			
			str = "\r\n\r\n<html><head></head><body>404 File not found<br/>Please check your url,and try it again!</body></html>";
			len = strlen(str);
			int htmllen = len;
			char num_len[5];
			memset(num_len, '\0', sizeof(num_len));
			sprintf(num_len, "%d", len);
            
			len = strlen(num_len);
			memcpy(send + index, num_len, len);
			index += len;
            
			memcpy(send_buf + index, str, htmllen);
			index += htmllen;
			break;
            
            
		default:
			break;
            
	}
	return index;
}
int reply_normal_information(unsigned char *send_buf, unsigned char *file_buf, int file_size,  char *mime_type)
{
	char *str =  "HTTP/1.1 200 OK\r\nServer:Mutu/Linux(0.1)\r\nDate:";
	register int index = strlen(str);
	memcpy(send_buf, str, index);
    
	char time_buf[TIME_BUFFER_SIZE];
	memset(time_buf, '\0', sizeof(time_buf));
	str = get_time_str(time_buf);
	int len = strlen(time_buf);
	memcpy(send_buf + index, time_buf, len);
	index += len;
    
	len = strlen(ALLOW);
	memcpy(send_buf + index, ALLOW, len);
	index += len;
    
	memcpy(send_buf + index, "\r\nContent-Type:", 15);
	index += 15;
	len = strlen(mime_type);
	memcpy(send_buf + index, mime_type, len);
	index += strlen(mime_type);
    
	memcpy(send_buf + index, "\r\nContent-Length:", 17);
	index += 17;
	char num_len[8];
	memset(num_len, '\0', sizeof(num_len));
	sprintf(num_len, "%d", file_size);
	len = strlen(num_len);
	memcpy(send_buf + index, num_len, len);
	index += len;
    
	memcpy(send_buf + index, "\r\n\r\n", 4);
	index += 4;
	
    
	memcpy(send_buf + index, file_buf, file_size);
	index += file_size;
	return index;
	
}







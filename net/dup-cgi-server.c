#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include "common.h"


int main(int argc, char *argv[])
{
    int reuse = 1;
    int fd = -1;
    int sockfd = -1, confd = -1;
    struct sockaddr_in addr = {};
    struct sockaddr_in cli_addr = {};
    socklen_t addr_len = sizeof(cli_addr);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(PF_INET, SERVER_IP, &addr.sin_addr);
    
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        LOG_ERROR("socket error.%s\n", strerror(errno));
        return -1;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        LOG_ERROR("setsockopt error.%s \n", strerror(errno));
        goto out;
    }
    
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr))){
        LOG_ERROR("bind error.%s\n", strerror(errno));
        goto out;
    }
    
    if(listen(sockfd, 5)){
        LOG_ERROR("bind error.%s\n", strerror(errno));
        goto out;
    }
    
    confd = accept(sockfd, (struct sockaddr*)&cli_addr, &addr_len);
    if(confd < 0){
        LOG_ERROR("confd error.%s\n", strerror(errno));
        goto out;
    }
    
    close(STDOUT_FILENO);
    dup(confd);

    //sleep (5);
    printf("Message from CGI server.\n");
    sleep (5);
    
out:
    // 描述符关闭太早的话，客户端接收不到printf 信息。
    close(sockfd);
    close(confd);
    //close(fd);
    
    return 0;
}
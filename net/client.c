#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "common.h"



static int stop = 0;

static void sig_hdl(int sig)
{
    stop = 1;
}


#if 0
    /*************************  Server step  *************************/
    // 创建socket  < linux 2.6.17起，type支持SOCK_NONBLOCK和SOCK_CLOEXEC对应值 >
    int socket(int domain,int type,int protocol);   
    // 命名socket 
    int bind(int sockfd,const struct sockaddr*my_addr,socklen_t addrlen);  
    // 监听socket
    int listen(int sockfd,int backlog);
    // 接受连接
    int accept(int sockfd,struct sockaddr*addr,socklen_t*addrlen);  
    // 关闭连接
    int close(int fd);
    int shutdown(int sockfd,int howto);
    
    // 数据读写
    ssize_t recv(int sockfd,void*buf,size_t len,int flags);
    ssize_t send(int sockfd,const void*buf,size_t len,int flags);
    
    int getsockopt(int sockfd,int level,int option_name,void*option_value,socklen_t*restrict option_len);
    int setsockopt(int sockfd,int level,int option_name,const void*option_value,socklen_t option_len);
    
    /*************************  Client step  *************************/
    // 创建socket  < linux 2.6.17起，type支持SOCK_NONBLOCK和SOCK_CLOEXEC对应值 >
    int socket(int domain,int type,int protocol);  
    // 发起连接
    int connect(int sockfd,const struct sockaddr*serv_addr,socklen_t addrlen);
    // 关闭连接
    int close(int fd);
    int shutdown(int sockfd,int howto);
    
    // 数据读写
    ssize_t recv(int sockfd,void*buf,size_t len,int flags);
    ssize_t send(int sockfd,const void*buf,size_t len,int flags);
    
#endif 

int main(int argc, char *argv[])
{
    int ret = -1;
    int sfd = -1;
    char buf[BUF_SIZE] = "";
    struct sockaddr_in ser_addr = {};
    socklen_t addr_len = sizeof(ser_addr);
    
    LOG("Client start !\n");
    signal(SIGTERM, sig_hdl);
    
    if((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        LOG("socker error.%s \n", strerror(errno));
        return -1;
    }
    
    
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &ser_addr.sin_addr);
    
    if(connect(sfd, (const struct sockaddr*)&ser_addr, addr_len)){
        LOG("connect error.%s \n", strerror(errno));
        return -1; 
    }
    
    sprintf(buf, "%s", "data from client");
    ret = send(sfd, buf, sizeof(buf), 0);
    if(ret <= 0){
        LOG("send error.%s \n", strerror(errno));
    }
    LOG("send success. byte=%d ", ret);
    
    bzero(buf, sizeof(buf));
    ret = recv(sfd, buf, sizeof(buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
    }
    LOG("recv success. byte=%d . %s \n", ret, buf);
    
    LOG("Client exit !\n");
    
    ret = 0;

    sleep(10);
    if(sfd > 0){
        close (sfd);
        //shutdown();
    }

    return ret;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "common.h"


#define MAX_EVENT_NUMBER 1024


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
    
    // 获取sockfd 连接本端地址
    int getsockname(int sockfd,struct sockaddr*address,socklen_t*address_len);
    int getpeername(int sockfd,struct sockaddr*address,socklen_t*address_len);
    
    int getsockopt(int sockfd,int level,int option_name,void*option_value,socklen_t*restrict option_len);
    int setsockopt(int sockfd,int level,int option_name,const void*option_value,socklen_t option_len);
    
    
    /* epoll */
    // 创建内核事件表
    int epoll_create(int size);
    // 操作内核事件表
    int epoll_ctl(int epfd,int op,int fd,struct epoll_event*event);
    
    
    
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


int do_worker(int confd)
{
    int ret = 0;
    char rcv_buf[BUF_SIZE] = "";
    char snd_buf[BUF_SIZE] = "";

    ret = recv(confd, rcv_buf, sizeof(rcv_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s .errno=%d  \n", strerror(errno), errno);
        //goto out;
    }
    LOG("recv:%s \n", rcv_buf);
    //bzero(snd_buf, sizeof(snd_buf));
    sprintf(snd_buf, "Respon from from server seq. \n");
    ret = send(confd, snd_buf, sizeof(snd_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
    }

    return 0;
}


 
int main(int argc, char *argv[])
{
    int ret = -1;
    int sfd = -1;
    int confd = -1;
    int i = -1;
    int reuse = 1;
    int num = 0;
    struct sockaddr_in addr = {};
    struct sockaddr_in client_addr = {};
    socklen_t addr_len = sizeof(client_addr);
    int port = 0;
    char peer_ip[INET_ADDRSTRLEN] = "";
    
    struct epoll_event events[MAX_EVENT_NUMBER];
    struct epoll_event event = {};
    
    
    
    LOG("Server start !\n");
    signal(SIGTERM, sig_hdl);
    
    if((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        LOG("socker error.%s \n", strerror(errno));
        return -1;
    }
    
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) {
        LOG_ERROR("setsockopt error.%s \n", strerror(errno));
        goto out;
    }
    
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    
    if(bind(sfd, (const struct sockaddr*)&addr, sizeof(addr))){
        LOG("bind error.%s \n", strerror(errno));
        goto out; 
    }
   
    if(listen(sfd, 5)){
        LOG("listen error.%s \n", strerror(errno));
        goto out; 
    }
    
    // int epoll_create(int size)
    // int epoll_ctl(int epfd,int op,int fd,struct epoll_event*event)
    // int epoll_wait(int epfd,struct epoll_event*events,int maxevents,int timeout);
    
    
    int epfd = epoll_create(5);

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLOUT;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sfd,  &event);   
    num = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
    
    
    while(!stop) { 
        
        for(i=0; i<num; i++){
            if(events[i].data.fd == sfd){
                confd = accept(sfd, (struct sockaddr*)&client_addr, &addr_len);
                if(confd < 0){
                    LOG("accept error.%s \n", strerror(errno));
                    goto out;
                }
                inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN);
                port = ntohs(client_addr.sin_port);
                LOG("accept confd=%d --> %s:%d  \n", confd, peer_ip, port);
                
                bzero(&event, sizeof(struct epoll_event));
                event.data.fd = confd;
                event.events = EPOLLIN | EPOLLOUT;
                epoll_ctl(epfd, EPOLL_CTL_ADD, confd, &event);
            } else {
                LOG("event coming... fd=%d \n", event.data.fd);
                do_worker(confd);
            }
            
        }
    }
    
    LOG("Server exit !\n");

    sleep (5);
    ret = 0;
out:
    if(sfd > 0){
        close (sfd);
    }
    return ret;
}

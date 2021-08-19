#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
    
    // 获取sockfd 连接本端地址
    int getsockname(int sockfd,struct sockaddr*address,socklen_t*address_len);
    int getpeername(int sockfd,struct sockaddr*address,socklen_t*address_len);
    
    int getsockopt(int sockfd,int level,int option_name,void*option_value,socklen_t*restrict option_len);
    int setsockopt(int sockfd,int level,int option_name,const void*option_value,socklen_t option_len);
	
	// 创建内核事件表
	int epoll_create(int size);
	// 操作epoll的内核事件表
	int epoll_ctl(int epfd,int op,int fd,struct epoll_event*event)
	// 在指定超时时间内等待一组文件描述符上的事件
	int epoll_wait(int epfd,struct epoll_event*events,int maxevents,int timeout);
    
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
    int confd = -1;
    int reuse = 1;
    struct sockaddr_in addr = {};
    struct sockaddr_in client_addr = {};
    socklen_t addr_len = sizeof(client_addr);
    char peer_ip[INET_ADDRSTRLEN] = ""; 
    char buf[BUF_SIZE] = "";
	int epfd = -1;
	struct epoll_event events = {}; 
    
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
    
    confd = accept(sfd, (struct sockaddr *)&client_addr, &addr_len);
    if(confd < 0){
        LOG("accept error.%s \n", strerror(errno));
        goto out; 
    } 
    
    //inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN);
    LOG("connected with client. client IP:%s PORT=%d \n", 
        inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN), ntohs(client_addr.sin_port));
    
    bzero(&client_addr, sizeof(client_addr));
    bzero(peer_ip, sizeof(peer_ip));
    if(getpeername(confd, (struct sockaddr*)&client_addr, &addr_len)){
         LOG("getpeername error.%s ", strerror(errno));
         goto out;
    }
    
    LOG("peer IP:%s PORT=%d \n", 
        inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN), ntohs(client_addr.sin_port));

	epfd = epoll_create(10);
	if(epfd < 0){
		LOG("epoll_create error.");
		goto out;
	}
	
	event.events = EPOLLIN|EPOLLOUT;
	event.data.fd = confdl;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, confd, &event);
	if(ret < -1){
		LOG("epoll_ctl error.\n");
		close(epfd);
		goto out;
	}
	ret = epoll_wait(epfd, &);

	int epoll_wait(int epfd,struct epoll_event*events,int maxevents,int timeout);
		
#if 0
    sprintf(buf, "%s", "data from server");
    ret = send(confd, buf, sizeof(buf), 0);
    if(ret <= 0){
        LOG("send error.%s \n", strerror(errno));
    }
    LOG("send success. byte=%d ", ret);
    
    bzero(buf, sizeof(buf));
    ret = recv(confd, buf, sizeof(buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
    }
    LOG("recv from client success. byte=%d : %s\n", ret, buf);
#endif     
    LOG("Server exit !\n");
    
 
    sleep (5);
    ret = 0;
out:
    if(sfd > 0){
        close (sfd);
    }
    if(confd > 0){
        close (sfd);
    }
    return ret;
}

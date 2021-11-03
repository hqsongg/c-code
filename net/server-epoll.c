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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"


#define MAX_EVENT_NUMBER 1024
static int stop = 0;


int event_ctl(int epfd, int op, int fd, int events);
int do_worker_lt(int confd);
int do_worker_et(int confd);
int setnoblocking(int fd);
int event_ctl(int epfd, int op, int fd, int events);





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
    // 一段时间内等待描述符
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




int event_ctl(int epfd, int op, int fd, int events)
{
    struct epoll_event event = {};
    
    event.data.fd = fd;
    event.events = events;

    return epoll_ctl(epfd, op, fd, &event);
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
    /* 为了防止向Client 已经关闭的描述符写操内核产生SIGPIPE信号导致服务器退出，需要对SIGPIP信号忽略 */
    signal(SIGPIPE,SIG_IGN);
    
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
    if((ret = event_ctl(epfd, EPOLL_CTL_ADD, sfd, EPOLLIN))){
        LOG("event_ctl error.%s \n", strerror(errno)); 
        goto out;
    }
    
    
    while(!stop) { 
        num = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        //LOG("event num:%d \n", num);
        for(i=0; i<num; i++){
            if(events[i].data.fd == sfd){
                confd = accept(sfd, (struct sockaddr*)&client_addr, &addr_len);
                if(confd < 0){
                    LOG("accept error.%s \n", strerror(errno));
                    continue;
                    //新的连接失败，不退出，保证已连接客户端继续工作
                    //goto out;
                }
                inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN);
                port = ntohs(client_addr.sin_port);
                LOG("accept confd=%d --> %s:%d  \n", confd, peer_ip, port); 
                setnoblocking(confd);
                event_ctl(epfd, EPOLL_CTL_ADD, confd, EPOLLIN|EPOLLET);
            } else {
                if(events[i].events[i] & EPOLLIN){
                    do_worker_et(events[i].data.fd);
                    //do_worker_lt(events[i].data.fd);
                }
                
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


int do_worker_lt(int confd)
{
    int ret = 0;
    char rcv_buf[BUF_SIZE] = "";
    char snd_buf[BUF_SIZE] = "";

    /* 默认工作在 LT模式（电平触发）只要socket缓存中还有数据未被读出，事件就会触发 */
    ret = recv(confd, rcv_buf, sizeof(rcv_buf), 0);
    if(ret <= 0){
        /*对于非阻塞IO， 下面的条件成立表示数据已经全部读取完毕。 此后， epoll就能再次触发sockfd上的EPOLLIN事件， 以驱动下一次读操作*/
        if((errno==EAGAIN)||(errno==EWOULDBLOCK)){
            printf("read later\n");
            goto out;
        }
        event_ctl(confd, EPOLL_CTL_DEL, confd, EPOLLIN);
        close(confd);
        LOG("recv error.%s .errno=%d  \n", strerror(errno), errno);
        goto out;
    }
    LOG("recv:%s  fd=%d \n", rcv_buf, confd);
    //bzero(snd_buf, sizeof(snd_buf));
    sprintf(snd_buf, "Respon from from server seq. \n");
    ret = send(confd, snd_buf, sizeof(snd_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
    }

out:
    return 0;
}

int do_worker_et(int confd)
{
    int ret = 0;
    char rcv_buf[BUF_SIZE] = "";
    char snd_buf[BUF_SIZE] = "";

     /* 工作ET模式,事件不会被重复触发，所以需要通过循环确保把缓存中数据完全读出 */
     while(1){
        bzero(rcv_buf, BUF_SIZE);
        ret = recv(confd, rcv_buf, sizeof(rcv_buf), 0);
        if(ret <= 0){
            /*对于非阻塞IO， 下面的条件成立表示数据已经全部读取完毕。 此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作*/
            if((errno==EAGAIN)||(errno==EWOULDBLOCK)){
                //printf("read later ret=%d \n", ret);
                break;
            }
            if(ret == 0){
                ret = event_ctl(confd, EPOLL_CTL_DEL, confd, EPOLLIN);
                if(ret){
                    LOG("event_ctl del error.%s\n", strerror(errno));
                }
                close(confd);
                LOG("close confd=%d \n", confd);
            }
            
            LOG("recv error.%s .errno=%d ret=%d \n", strerror(errno), errno, ret);
            break;
        }
        LOG("recv:%s  fd=%d \n", rcv_buf, confd);
     }
    
    bzero(snd_buf, sizeof(snd_buf));
    sprintf(snd_buf, "Respon from from server seq. \n");
    ret = send(confd, snd_buf, sizeof(snd_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
    }

out:
    return 0;
}

int setnoblocking(int fd)
{
    int new_opt = 0;
    int old_opt = 0;
    
    old_opt = fcntl(fd, F_GETFL);
    new_opt = old_opt|O_NONBLOCK;
    
    fcntl(fd, F_SETFL, new_opt);
    
    return 0;
}

/* EPOLLONESHOT 可以防止ET模式下，一个线程在读取数据过程中，
 *因数据到达而引发的再次触发，导致连个线程同时操作一个socket问题 
*/
void reset_oneshot(int epfd, int fd)
{
    struct epoll_event event = {};
    
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLONESHOT|O_NONBLOCK;

    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}

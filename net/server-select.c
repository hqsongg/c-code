#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
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
    
    
    int select(int nfds,fd_set*readfds,fd_set*writefds,fd_set*exceptfds,struct timeval*timeout);
    
    
    
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


int do_worker(int confd, fd_set *prfds)
{
    int ret = 0;
    char rcv_buf[BUF_SIZE] = "";
    char snd_buf[BUF_SIZE] = "";

    ret = recv(confd, rcv_buf, sizeof(rcv_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s .errno=%d  \n", strerror(errno), errno);
        //goto out;
        close(confd);
        FD_CLR(confd, prfds);
    }

    //bzero(snd_buf, sizeof(snd_buf));
    sprintf(snd_buf, "Respon from from server seq=%s. \n", rcv_buf);
    ret = send(confd, snd_buf, sizeof(snd_buf), 0);
    if(ret <= 0){
        LOG("recv error.%s \n", strerror(errno));
        close(confd);
        FD_CLR(confd, prfds);
    }

    return 0;
}

 
int main(int argc, char *argv[])
{
    int ret = -1;
    int sfd = -1;
    int confd = -1;
    int maxfd = -1;
    int i = -1;
    int reuse = 1;
    struct sockaddr_in addr = {};
    struct sockaddr_in client_addr = {};
    socklen_t addr_len = sizeof(client_addr);
    int port = 0;
    char peer_ip[INET_ADDRSTRLEN] = "";
    
    fd_set read_fds;
    fd_set write_fds;
    fd_set exception_fds;
    fd_set tmp_rfds;
    fd_set tmp_wfds;
    fd_set tmp_xfds;
    
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
    
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    //FD_ZERO(&exception_fds);
    FD_SET(sfd, &read_fds);
    FD_SET(sfd, &write_fds);
    //FD_SET(sfd, &exception_fds);
    tmp_rfds = read_fds;
    tmp_wfds = write_fds;
    
    maxfd = sfd;

    
    while(!stop) { 
        //LOG("call select ... \n");
        
        // 重新设置描述符集合
        read_fds = tmp_rfds;
        write_fds = tmp_wfds;
        
        /* select将更新这个集合,把其中不可读的套节字去掉,
         * 只保留符合条件的套节字在这个集合里面，故每次需要重新设置集合
        */
        ret = select(maxfd+1, &read_fds, NULL, NULL, 0);
        if (ret < 0){
            LOG("select error. %s \n", strerror(errno));
            goto out;
        }
        
        //判断sfd是否在可读集合中
        if(FD_ISSET(sfd, &read_fds)){
            confd = accept(sfd, (struct sockaddr*)&client_addr, &addr_len);
            if(confd < 0){
                LOG("accept error.%s \n", strerror(errno));
                goto out;
            }
            inet_ntop(AF_INET, &client_addr.sin_addr, peer_ip, INET_ADDRSTRLEN);
            port = ntohs(client_addr.sin_port);
            LOG("accept confd=%d --> %s:%d  \n", confd, peer_ip, port);
            
            //将新的连接描述符加入到监听集合中
            FD_SET(confd, &tmp_rfds);
            maxfd = (confd > maxfd) ? confd : maxfd;
        }
        
        for(i=sfd+1; i<=maxfd; i++){
            if(FD_ISSET(i, &read_fds)){
                 do_worker(i, &tmp_rfds);
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
    if(confd > 0){
        close (sfd);
    }
    return ret;
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"


    /* 往读端关闭的管道写，会产生SIGPIPE (数值：141 ？)信号,
     * 往写段关闭的管道读数据会直接返回0.
     SIGPIPE信号产生的规则：
        当一个进程向某个已收到RST的套接字执行写操作时，内核向该进程发送SIGPIPE信号。
     总结：对于C/S 模型。为了防止客户端进程终止，而导致服务器进程被SIGPIPE信号终止，
            因此服务器程序要处理SIGPIPE信号。
      参考：https://blog.csdn.net/weixin_36750623/article/details/91370604
     */

int main(int argc, char *argv[])
{
    int ret = -1;
    int pipefd[2] = {};
    char wbuf[BUF_SIZE] = "hello word !!";
    char rbuf[BUF_SIZE] = "";
    
    if((ret = pipe(pipefd))){
        LOG_ERROR("pipe error. %s\n", strerror(errno));
        return -1;
    }
    
    // 关闭读/写描述符。
    //close(pipefd[1]);
    
#if 1
    ret = write(pipefd[1], wbuf, sizeof(wbuf));
    if(ret <= 0){
        LOG_ERROR("write error. %s\n", strerror(errno));
        goto out;
    }
    LOG_DEBUG("Write success %d byte. wbuf:%s \n", ret, wbuf);

#else   
    ret = read(pipefd[0], rbuf, sizeof(rbuf));
    if(ret < 0){
        LOG_ERROR("read error. %s\n", strerror(errno));
        goto out;
    }
    LOG_DEBUG("Read success %d byte. rbuf:%s \n", ret, rbuf);
#endif 

out:
    if (pipefd[0]){
        close(pipefd[0]);
    }
    if (pipefd[1]){
        close(pipefd[1]);
    }
    return ret;
}

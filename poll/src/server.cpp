#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>

#include <iostream>

#define PORT 9999
#define MAX_FD_SIZE 1024

int main()
{
    // 1.创建套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
    {
        std::cout <<"创建套接字失败\n";
        exit(0);
    }
    // 2. 绑定 ip, port
    struct sockaddr_in addr;
    addr.sin_port = htons(9999);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        std::cout <<"绑定套接字失败\n";
        exit(0);
    }
    // 3. 监听
    ret = listen(lfd, 100);
    if(ret == -1)
    {
        std::cout <<"设置监听失败\n";
        exit(0);
    }
    
    // 4. 等待连接 -> 循环
    // 检测 -> 读缓冲区, 委托内核去处理
    // 数据初始化, 创建自定义的文件描述符集
    struct pollfd fds[MAX_FD_SIZE];
    // 初始化
    for(int i=0; i<MAX_FD_SIZE; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }
    fds[0].fd = lfd;

    int maxfd = lfd;
    while(1)
    {
        // 委托内核检测
        ret = poll(fds, maxfd+1, -1);   // -1: 一直阻塞
        if(ret == -1)
        {
            std::cout <<"poll error\n";
            exit(0);
        }

        // 检测的度缓冲区有变化
        // 有新连接
        if(fds[0].revents & POLLIN)     // 监听套接字 结构体
        {
            // 接收连接请求
            struct sockaddr_in sockcli;
            socklen_t len = sizeof(sockcli);
            // 这个accept是不会阻塞的
            // 此处已经确定 lfd 处收到数据才执行
            int connfd = accept(lfd, (struct sockaddr*)&sockcli, &len);
            // 委托内核检测connfd的读缓冲区
            int i;
            for(i=0; i<MAX_FD_SIZE; ++i)
            {
                if(fds[i].fd == -1)
                {
                    fds[i].fd = connfd;
                    break;
                }
            }
            maxfd = i > maxfd ? i : maxfd;
        }
        // 通信, 有客户端发送数据过来
        for(int i=1; i<=maxfd; ++i)
        {
            // 如果在集合中, 说明读缓冲区有数据
            if(fds[i].revents & POLLIN)
            {
                char buf[128];
                int ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if(ret == -1)
                {
                    std::cout <<"recv error\n";
                    exit(0);
                }
                else if(ret == 0)
                {
                    std::cout <<"客户端已经关闭了连接...\n";
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                else
                {
                    std::cout <<"recv message: "<< buf <<"\n";
                    int msgLen = strlen(buf);
                    for (int i = 0; i < strlen(buf); i ++ )
                    {
                        if (buf[i] >= 'a' && buf[i] <= 'z')
                            buf[i] = buf[i] + ('A' - 'a');
                    }
                    std::cout <<"send message: "<< buf <<"\n";
                    send(fds[i].fd, buf, strlen(buf)+1, 0);
                }
            }
        }
    }
    close(lfd);
    return 0;
}
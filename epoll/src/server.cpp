#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/poll.h>
#include <sys/epoll.h>

#include <iostream>
#include <thread>

#define PORT 9999
#define MAX_FD_SIZE 1024

void acceptConn(int fd, int ep_fd)
{
    std::cout <<"acceptConn tid : " << pthread_self() <<'\n';
    int cfd = accept(fd, NULL, NULL);

    // 设置为非阻塞
    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // 数据传输到内核，会进行一次拷贝，因此只需要一个变量临时存储，再传输到内核即可
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = cfd;
    int ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, cfd, &ev);
}

void communication(int fd, int ep_fd)
{
    std::cout <<"communication tid : " << pthread_self() <<'\n';
    char buff[5];
    char tmp[1024];
    // bzero(tmp, sizeof(tmp));
    memset(tmp, 0, sizeof(tmp));
    std::cout <<"------\n";
    while (1)
    {
        int ret = recv(fd, buff, sizeof(buff), 0);
        if (ret == -1){
            if (errno == EAGAIN){
                std::cout <<"数据已经接收完毕\n";
                send(fd, tmp, strlen(tmp) + 1, 0);
                break;
            }
            else {
                std::cout <<"recv error\n";
                break;
            }
        }
        else if (ret == 0){
            std::cout <<"客户端断开连接\n";
            // 一定要，先删除，再关闭文件描述符，否则会导致epoll_ctl删除失败
            epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            break;
        }
        else 
        {
            std::cout <<"recv message: "<< buff <<"\n";
            int msgLen = strlen(buff);
            for (int i = 0; i < strlen(buff); i ++ )
            {
                if (buff[i] >= 'a' && buff[i] <= 'z')
                    buff[i] = buff[i] + ('A' - 'a');
            }
            strncat(tmp  + strlen(tmp), buff, ret);
            // std::cout <<"change message: "<< buff + '\0' <<"\n";
            // send(fd, buff, strlen(buff)+1, 0);
        }
    }
}

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

    // 4. 创建 epoll 实例
    int ep_fd = epoll_create(1);
    if (ep_fd == -1)
    {
        std::cout <<"创建epoll实例失败\n";
        exit(0);
    }
    // 5. 监听套接字上树
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = lfd;
    ret = epoll_ctl(ep_fd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1)
    {
        std::cout <<"EPOLL_CTL_ADD 执行失败\n";
        exit(0);
    }

    // 6. 循环监听
    struct epoll_event evs[1024];
    int size = 1024;
    while(1)
    {
        int count = epoll_wait(ep_fd, evs, size, -1);
        // std::cout <<"count : "<< count <<'\n';
        for (int i = 0; i < count; i++)
        {
            int fd = evs[i].data.fd;
            if (fd == lfd) {    // 监听文件描述符
                std::thread listenThread(acceptConn, fd, ep_fd);
                listenThread.detach();
            }else {
                std::thread cliThread(communication, fd, ep_fd);
                cliThread.detach();
            }
        }
    }
    close(lfd);
    return 0;
}
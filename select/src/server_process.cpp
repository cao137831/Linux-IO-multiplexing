#include <iostream>
#include <strings.h>
#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/wait.h>

#include <thread>

#define PORT 8001
#define MESSAGE_LEN 1024
#define FD_SIZE 1024

int main()
{
    int ret = -1;
    int backlog = 10;   // 缓冲长度
    int socket_fd, accept_fd;
    struct sockaddr_in localaddr, remoteaddr;
    char buff[MESSAGE_LEN] = {0, };
    pid_t pid = -1;
    /* socket */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cout <<"Failed to create socket!\n";
        exit(-1);
    }

    /* bind */
    localaddr.sin_addr.s_addr = INADDR_ANY;
    localaddr.sin_port = PORT;
    localaddr.sin_family = AF_INET;
    bzero(&(localaddr.sin_zero), 0);
    ret = bind(socket_fd, (const sockaddr*)&localaddr,sizeof(localaddr));
    if (ret == -1){
        std::cout <<"Failed to bind addr!\n";
        exit(-1);
    }

    ret = listen(socket_fd, backlog);
    if (ret == -1){
        std::cout <<"Failed to listen addr!\n";
        exit(-1);
    }


    while (true)
    {
        socklen_t addr_len = sizeof(struct sockaddr);
        accept_fd = accept(socket_fd,(struct sockaddr*)&remoteaddr,&addr_len);  // 非阻塞
        if (accept_fd == -1){
            std::cout <<"Failed to accept!\n";
            break;
        }
        /* 创建多进程 */
        pid = fork();
        if (pid == -1) {
            std::cout <<"reate new process failure: "<< strerror(errno) <<"\n";
            close(accept_fd);
        }
        else if (pid > 0){  // father 进程
            close(accept_fd);
            continue;
        }
        else if (pid == 0){  //  child 进程
            while(true){
                memset(buff, 0, MESSAGE_LEN);
                int ret = recv(accept_fd,buff,MESSAGE_LEN, 0);
                if (ret == 0){  // 返回值 0， 表示客户端断开连接
                    close(accept_fd);
                    break;
                }
                std::cout <<"recv : "<< buff <<'\n';
                for (int i = 0; i < strlen(buff); i ++ )
                {
                    if (buff[i] >= 'a' && buff[i] <= 'z')
                        buff[i] = buff[i] + ('A' - 'a');
                }
                std::cout <<"send : "<< buff <<'\n'<<"------------------------\n";
                send(accept_fd,buff,MESSAGE_LEN,0);
            }
        }
    }
    close(socket_fd);
    return 0;
}
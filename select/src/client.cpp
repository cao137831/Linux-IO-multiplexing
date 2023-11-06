#include <iostream>
#include <strings.h>
#include <cstring>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8001
#define MESSAGE_LEN 1024

int main(int argc, char* argv[])
{
    int ret = -1;
    int socket_fd, accept_fd;
    struct sockaddr_in serveraddr;
    char sendbuf[MESSAGE_LEN] = {0, };
    char recvbuf[MESSAGE_LEN] = {0, };

    /* socket */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cout <<"Failed to create socket!\n";
        exit(-1);
    }

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = PORT;
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* connect */
    ret = connect(socket_fd, (struct sockaddr*)&serveraddr,sizeof(struct sockaddr));
    if (ret == -1){
        std::cout <<"Failed to connect server!\n";
        exit(-1);
    }

    for (;;){
        std::cin.getline(sendbuf, MESSAGE_LEN);
        /* send */
        ret = send(socket_fd, sendbuf, strlen(sendbuf), 0);
        if (ret == -1){
            std::cout <<"Failed to send data!\n";
            break;
        }

        if (strcmp(sendbuf, "quit") == 0){
            std::cout <<"断开连接!\n";
            break;
        }

        /* recv */
        ret = recv(socket_fd, recvbuf, MESSAGE_LEN, 0);
        recvbuf[ret] = '\0';
        std::cout <<"服务器返回的数据: "<< recvbuf <<'\n';
        if (ret == -1){
            std::cout <<"Failed to recv data!\n";
            break;
        }
    }
    /* close */
    close(socket_fd);

    return 0;
}
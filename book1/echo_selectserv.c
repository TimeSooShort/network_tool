#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100

void error_handling(char *message);

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage %s <port> \n", argv[0]);
        exit(1);
    }

    int socket_cln, socket_serv;
    socklen_t sock_info_sz, optln;
    struct sockaddr_in sock_info_cln, sock_info_serv;
    fd_set reads, reads_tmp;
    int fd_max, fd_num;
    struct timeval timeout;
    char buf[BUF_SIZE];
    int recv_len;
    int option;
    char *shut_return = "Ok, I know you will close, bye!";

    socket_serv = socket(PF_INET, SOCK_STREAM, 0);
    if(socket_serv == -1)
    {
        error_handling("socket() error");
    }

    optln = sizeof(option);
    option = 1;
    setsockopt(socket_serv, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optln);

    memset(&sock_info_serv, 0, sizeof(sock_info_serv));
    sock_info_serv.sin_family = AF_INET;
    sock_info_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_info_serv.sin_port = htons(atoi(argv[1]));
    if(bind(socket_serv, (struct sockaddr*)&sock_info_serv, sizeof(sock_info_serv)) == -1)
    {
        error_handling("bind() error");
    }

    if(listen(socket_serv, 5) == -1)
    {
        error_handling("listen() error");
    }

    FD_ZERO(&reads);
    FD_SET(socket_serv, &reads);
    fd_max = socket_serv;

    while(1)
    {
        reads_tmp = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if((fd_num = select(fd_max+1, &reads_tmp, 0, 0, &timeout)) == -1)
        {
            printf("select() error \n");
            break;
        }
        if(fd_num == 0)
        {
            printf("no socket events happen\n");
            continue;
        }

        for(int i = 0; i < fd_max+1; ++i)
        {
            if(FD_ISSET(i, &reads_tmp))
            {
                if(i == socket_serv)
                {
                    sock_info_sz = sizeof(sock_info_cln);
                    socket_cln = accept(socket_serv, (struct sockaddr*)&sock_info_cln, &sock_info_sz);
                    if(socket_cln == -1)
                    {
                        continue;
                    }
                    FD_SET(socket_cln, &reads);
                    if(socket_cln > fd_max)
                    {
                        fd_max = socket_cln;
                    }
                    printf("Connected client: %d \n", socket_cln);
                }
                else
                {
                    recv_len = read(i, buf, BUF_SIZE);
                    if(recv_len == 0)
                    {
                        write(i, shut_return, strlen(shut_return));
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else
                    {
                        write(i, buf, recv_len);
                    }
                }
            }
        }
    }
    close(socket_serv);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

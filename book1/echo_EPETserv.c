#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define EPOLL_SIZE 50

void error_handling(char *message);
void setnonblockingmode(int fd);

// Edge Trigger
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage %s <port> \n", argv[0]);
        exit(1);
    }

    struct sockaddr_in addr_cln, addr_serv;
    socklen_t addr_cln_sz, optln;
    char *buf[BUF_SIZE];
    int read_ln;
    int sock_cln, sock_serv;
    int option;
    
    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    sock_serv = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_serv == -1)
    {
        error_handling("socket() error");
    }

    optln = sizeof(option);
    option = 1;
    setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optln);

    memset(&sock_serv, 0, sizeof(sock_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_serv.sin_port = htons(atoi(argv[1]));
    if(bind(sock_serv, (struct sockaddr*)&addr_serv, sizeof(addr_serv)) == -1)
    {
        error_handling("bind() error");
    }

    if(listen(sock_serv, 5) == -1)
    {
        error_handling("listen() error");
    }

    epfd = epoll_create(EPOLL_SIZE); // useless parameter
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    setnonblockingmode(sock_serv);
    event.events = EPOLLIN;
    event.data.fd = sock_serv;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_serv, &event);

    while(1)
    {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1)
        {
            puts("epoll_wait() error");
            break;
        }
        for(int i = 0; i < event_cnt; ++i)
        {
            if(ep_events[i].data.fd == sock_serv)
            {
                addr_cln_sz = sizeof(addr_cln);
                sock_cln = accept(sock_serv, (struct sockaddr*)&addr_cln, &addr_cln_sz);
                if(sock_cln == -1)
                {
                    puts("accept() error");
                    break;
                }
                setnonblockingmode(sock_cln);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = sock_cln;
                epoll_ctl(epfd, EPOLL_CTL_ADD, sock_cln, &event);
                printf("connected client: %d \n", sock_cln);
            }
            else
            {
                while(1)
                {
                    read_ln = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if(read_ln == 0)
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                        close(ep_events[i].data.fd);
                        printf("closed client: %d \n", ep_events[i].data.fd);
                        break;
                    }
                    else if(read_ln < 0)
                    {
                        if(errno == EAGAIN) // means already read all datas in read buffer
                        {
                            break;
                        }
                    }
                    else
                    {
                        write(ep_events[i].data.fd, buf, read_ln);
                    }
                }
            }
        }
    }
    close(sock_serv);
    close(epfd);
    return 0;
}

void setnonblockingmode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

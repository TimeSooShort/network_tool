#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void * handle_clnt(void *arg);
void error_handling(char *message);

pthread_mutex_t mutx;
int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage %s <port> \n", argv[0]);
        exit(1);
    }

    struct sockaddr_in addr_clnt, addr_serv;
    socklen_t addr_clnt_sz, opln;
    int sock_serv, sock_clnt;
    int option;
    pthread_t t_id;

    pthread_mutex_init(&mutx, NULL);

    sock_serv = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_serv == -1)
    {
        error_handling("socket() error");
    }

    opln = sizeof(option);
    option = 1;
    setsockopt(sock_serv, SOL_SOCKET, SO_REUSEADDR, (void*)&option, opln);

    memset(&addr_serv, 0, sizeof(addr_serv));
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

    while(1)
    {
        addr_clnt_sz = sizeof(addr_clnt);
        sock_clnt = accept(sock_serv, (struct sockaddr*)&addr_clnt, &addr_clnt_sz);
        if(sock_clnt == -1)
        {
            printf("accept() error");
            continue;
        }

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = sock_clnt;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id, NULL, handle_clnt, (void*)&sock_clnt);
        pthread_detach(t_id);
        printf("Connected client IP: %s \n", inet_ntoa(addr_clnt.sin_addr));
    }
    close(sock_serv);
    return 0;
}

void * handle_clnt(void *arg)
{
    int clnt_sock = *((int*)arg);
    int read_sz;
    char buf[BUF_SIZE];

    while((read_sz = read(clnt_sock, buf, BUF_SIZE)) != 0)
    {
        pthread_mutex_lock(&mutx);
        for(int i = 0; i < clnt_cnt; ++i)
        {
            write(clnt_socks[i], buf, read_sz);
        }
        pthread_mutex_unlock(&mutx);
    }
    pthread_mutex_lock(&mutx);
    for(int i = 0; i < clnt_cnt; ++i)
    {
        if(clnt_sock == clnt_socks[i])
        {
            while(i++ < clnt_cnt-1)
            {
                clnt_socks[i] = clnt_socks[i+1];
            }
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

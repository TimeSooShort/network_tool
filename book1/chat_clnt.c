#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define NAME_SIZE 20
#define BUF_SIZE 100

void error_handling(char *message);
void * send_msg(void *arg);
void * recv_msg(void *arg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in addr;
    pthread_t snd_thread, recv_thread;
    void *thread_return;

    if(argc != 4)
    {
        printf("usage %s <IP> <port> <name> \n", argv[0]);
        exit(0);
    }

    sprintf(name, "[%s]", argv[3]);

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        error_handling("connect() error");
        exit(1);
    }

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&recv_thread, NULL, recv_msg, (void*)&sock);

    pthread_join(snd_thread, &thread_return);
    pthread_join(recv_thread, &thread_return);

    close(sock);
    return 0;
}

void * send_msg(void *arg)
{
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1)
    {
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
        {
            close(sock);
            exit(0);
        }
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void * recv_msg(void *arg)
{
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    int read_sz;
    while(1)
    {
        read_sz = read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
        if(read_sz == -1)
        {
            return (void*)-1;
        }
        name_msg[read_sz] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

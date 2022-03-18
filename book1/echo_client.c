#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Usage : %s <IP> <PORT> \n", argv[0]);
        exit(1);
    }
    int sock;
    struct sockaddr_in sock_addr;
    char buf[BUF_SIZE];
    int read_sz, str_len, final_read_len;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) error_handling("socket() error...");
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sock_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1)
    {
        error_handling("connect() error");
    }
    puts("Connected....");

    while(1)
    {

        fputs("Inputs message(Q/q to quit): ", stdout);
        fgets(buf, BUF_SIZE, stdin);
        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            break;
        }
        str_len = write(sock, buf, strlen(buf));
        final_read_len = 0;
        while(final_read_len < str_len)
        {
            read_sz = read(sock, buf, BUF_SIZE);
            if(read_sz == -1)
            {
                error_handling("read() error");
            }
            else if(read_sz == 0)
            {
                puts("receive EOF");
                break;
            }
            final_read_len += read_sz; 
        }
        buf[final_read_len] = 0;
        printf("Message from server: %s", buf);
    }
    shutdown(sock, SHUT_WR);
    read(sock, buf, BUF_SIZE);
    printf("The Message form server When client shotdown: %s \n", buf);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

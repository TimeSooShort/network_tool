#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h> // for memset

#define BUF_SIZE 1024
#define OPSZ 4
int calculate(int opnm, int opnds[], char op);
void error_handing(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    int i, clnt_adr_sz, opnd_cnt, recv_len, recv_cnt, result;
    char opinfo[BUF_SIZE];
    if(argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock ==-1)
        error_handing("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    //serv_adr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
    {
        error_handing("socket() error");
    }
    puts("already binded");
    if(listen(serv_sock, 5)==-1)
        error_handing("listen() error");
    puts("listening....");

    clnt_adr_sz = sizeof(clnt_adr);
    for(i=0; i<2; i++)
    {
        printf("round %d start!\n", i);
        opnd_cnt = 0;
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        read(clnt_sock, &opnd_cnt, 1);
        
        recv_len = 0;
        while((opnd_cnt*OPSZ+1)>recv_len)
        {
             recv_cnt=read(clnt_sock, &opinfo[recv_len], BUF_SIZE-1);
             recv_len+=recv_cnt;
        }
        result=calculate(opnd_cnt, (int*)opinfo, opinfo[recv_len-1]);
        write(clnt_sock, (char*)&result, sizeof(result));
        close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}

void error_handing(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int calculate(int opnm, int opnds[], char op)
{
    printf("opnm: %d,  op: %c", opnm, op);
    int result = opnds[0], i;
    switch(op)
    {
    case '+':
        for(i = 1; i<opnm; i++) result+=opnds[i];
        break;
    case '-':
        for(i = 1; i<opnm; i++) result-=opnds[i];
        break;
    case '*':
        for(i = 1; i<opnm; i++) result*=opnds[i];
        break;
    }
    return result;
}


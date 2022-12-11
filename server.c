#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAXLINE 4096
#define PORT 8080
#define LISTENQ 1024
 
struct sockaddr_in servaddr, cliaddr;

int main (int argc, char **argv){
    int connfd, sockfd, n;
    char buff[MAXLINE+1];
    char oper;
    char message[] = "Podaj dzialanie(np. 2 + 2): ";
    socklen_t clilen;
    pid_t childpid;
    
    if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0){
        printf("Blad utworzenia polaczenia\n");
        exit(0);
    }
    
    bzero(buff,sizeof(buff));
    bzero(&servaddr, sizeof(servaddr));
      
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
        
    if((bind(sockfd, (struct servaddr *)&servaddr, sizeof(servaddr))) != 0){
        perror("Bind failed");
        exit(0);
    }
        
    listen(sockfd,LISTENQ);
    while(1)
    {
        clilen = sizeof(cliaddr);
        connfd = accept(sockfd,(struct sockaddr *) &cliaddr, &clilen);
        if (connfd < 0)
        {
            printf("Blad akceptacji polaczenia: %s\n", strerror(errno));
        }
        
        read(connfd, buff, sizeof(buff));
        printf("%s",buff);
        bzero(buff,sizeof(buff));
        while((buff[n++]=getchar())!='\n');
        printf("odp: %s\n", buff);
        if (buff[0] == 'q')
        {
            n = 0;
            close(connfd);
            return 0;
        }
        else
        {
            n = 0;
            write(connfd, buff, sizeof(buff));
            bzero(buff,sizeof(buff));
            close(connfd);
        }
    }
}

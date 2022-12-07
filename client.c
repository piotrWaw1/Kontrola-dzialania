#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>
#define MAXLINE 4096
#define PORT 8080
#define LISTENQ 1024

struct sockaddr_in servaddr;
// void sig_chld(int signo);

int main(int argc, char **argv){
    int sockfd, connfd;
    int n = 0; // iterator po adresach
    int status;
    char buff[MAXLINE + 1];
    pid_t childpid [2];
    childpid[0] = 0;
    childpid[1] = 0;

    int buysy_addres [2];

    char *ip [2]; // lista ip
    ip[0] = "127.0.0.1"; // Ip maszyny wirtualnej
    ip[1] = "192.168.0.125"; // Ip komputera
    
    // signal(SIGCHLD, sig_chld);
    
    while(1){
        if (childpid[n] == 0)
        {
            if((childpid[n] = fork())==0)
            {
                if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
                {
                    printf("Blad utworzenia polaczenia\n");
                    exit(0);
                }

                if(inet_pton(AF_INET, ip[n], &servaddr.sin_addr)<=0)
                {
                    printf("Blad konwersji do adresu IP dla %s\n", argv[1]);
                    exit(0);
                }
                
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_family = AF_INET;
                servaddr.sin_port = htons(PORT);
                servaddr.sin_addr.s_addr = inet_addr(ip[n]);
                
                // sprawdza czy udao sie polaczyc z klientem
                if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))!=0) 
                {
                    printf("Klient nie dziala: %d\n", n);
                    close(sockfd);
                    exit(0);
                }
                
                /*jezeli polaczenie sie uda sprawdza czy uzytkownik pracuje
                wysyla proste dzialanie 2+2 i oczekuje na odpowiedz 4*/
                else
                {
                    printf("Klient dziala: %d\n", n);
                    
                    write(sockfd, "2+2 = ", sizeof("2+2 = "));
                    read(sockfd, buff, sizeof(buff));
                    printf("Odp: %s\n",buff);
                    if (buff[0] == '4')
                    {
                        printf("Poprawna odpowiedz uzytkownik pracuje\n");
                    }
                    else if(buff[0] == ' ')
                    {
                        printf("Brak odpowiedzi\n");
                    }
                    else
                    {
                        printf("Niepoprawna odpowiedz.\n");
                    }
                    close(sockfd);
                    exit(0);
                }
            }
        }
        else if (waitpid(childpid[n], &status, WNOHANG) == childpid[n])
        {
            printf("reset: %d\n",n);
            childpid[n] = 0;
        }

        n++;
        if(n>1)
        {   
            n = 0;
            sleep(2);
        }
        
    } 
    
    return 0;
}

// void sig_chld(int signo){
//     pid_t pid;
//     int stat;
//     pid = wait(&stat);
//     printf("\nPotomek %d zakonczony\n", pid);
// }
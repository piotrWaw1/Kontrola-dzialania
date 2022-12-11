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
#include <time.h>
#define MAXLINE 4096
#define PORT 8080
#define LISTENQ 1024

int count_line();

int main(int argc, char* argv[]){
    struct sockaddr_in servaddr;

    int sockfd, connfd;
    int n = 0; // iterator po adresach
    int i = 0;
    int status;
    char buff[MAXLINE + 1];
    size_t len = 0;
    pid_t childpid;
    srand(time(NULL));
    
    if (argc < 2)
    {
        printf("Urzycie: ./client liczba_hostow\n");
        exit(0);
    }

    if (argv[1] <= 0)
    {
        printf("Zla liczba hostow\n");
        exit(0);
    }
    char *c = argv[1];
    int number_of_ip = atoi(c);

    char *ip [number_of_ip]; // lista ip
    for (int i = 0; i < number_of_ip; i++)
    {
        ip[i] = NULL;
    }
    // ip[0] = "127.0.0.1"; // Ip maszyny wirtualnej
    // ip[1] = "192.168.0.125"; // Ip komputera
    FILE *f = fopen("ip_list","r");
    if (f == NULL)
    {
        printf("Nie mozna odczytac pliku\n");
        exit(0);
    }
    
    while (getline(&ip[i], &len,f) != -1)
    {
        ip[i][strlen(ip[i])-1] = '\0';
        // printf("%s\n",ip[i]);
        if(i < number_of_ip)
        {
            i++;
        }
        if (i == number_of_ip)
        {
            break;
        }
    }
    
    // printf("Dziala\n");
    while(1){
        
        if((childpid = fork())==0)
        {
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
                
            if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
            {
                printf("Blad utworzenia polaczenia\n");
                exit(0);
            }
                
            // setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            if(inet_pton(AF_INET, ip[n], &servaddr.sin_addr)<=0)
            {
                printf("Blad konwersji do adresu IP dla %s\n", argv[1]);
                exit(0);
            }
                
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(PORT);
            servaddr.sin_addr.s_addr = inet_addr(ip[n]);
                
            // sprawdza czy udao sie polaczyc z klientem
            if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) 
            {
                printf("Klient nr %d nie dziala.\n", n);
                close(sockfd);
                exit(0);
            }

            /*jezeli polaczenie sie uda sprawdza czy uzytkownik pracuje
            wysyla proste dzialanie 2+2 i oczekuje na odpowiedz 4*/
            else
            {
                setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                printf("Klient nr %d dziala \n", n);
                    
                if (write(sockfd, "2+2 = ", sizeof("2+2 = ")) < sizeof("2+2 = ")) 
                {
                    printf("Blad wysylania\n");
                    close(sockfd);
                    exit(0);
                }

                int byte = recv(sockfd, buff, sizeof(buff), 0);
                if (byte == -1)
                {
                    // wystspil blad, sprawdź errno dla konkretnego bledu
                    if (errno == EAGAIN || errno == EWOULDBLOCK) // EGAIN i EWOULDBLOCK oznaczaja brak danych do odczytu
                    {
                        // uplyna czas oczekiwania
                        printf("Uplyna czas oczekiwania na dane\n");
                        close(sockfd);
                        exit(0);
                    }
                    else
                    {
                        // wystapil inny blad
                        printf("error podczas odczytu danych: %d\n", errno);
                        close(sockfd);
                        exit(0);
                    }
                }
                else
                {
                    // dana zostaly odebrane
                    printf("Odp: %s\n",buff);
                    if (buff[0] == '4')
                    {
                        printf("Poprawna odpowiedz uzytkownik pracuje\n");
                        close(sockfd);
                        exit(0);
                    }
                    else
                    {
                        printf("Niepoprawna odpowiedz.\n");
                        close(sockfd);
                        exit(0);
                    }
                }
            }
        }

        n++;
        if(n==number_of_ip)
        {   int min = 10;
            int max = 15;
            int random = 60 * (rand() % (max - min + 1) + min);// serwer zawisza dzialanie w czasie z przedzialu od 10 do 15 minut
            printf("Sleep: %d\n", random);
            n = 0;
            sleep(random);
        }
    } 
    
    return 0;
}

int count_line()
{
    FILE *f = fopen("ip_list","r");
    
    if (f == NULL)
    {
        printf("Nie mozna otworzyc pliku\n");
        exit(0);
    }

    int count_line;
    char c;

    for (c = getc(f); c != EOF; c = getc(f))
    {
        if (c == '\n')
        {
            count_line ++;
        }
    }
    fclose(f);

    return count_line;
}
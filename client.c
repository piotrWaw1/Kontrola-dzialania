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


int main(int argc, char* argv[]){
    struct sockaddr_in servaddr;

    int sockfd, connfd;
    int n = 0; // iterator po adresach
    int i = 0;
    char buff[MAXLINE + 1];
    size_t len = 0;
    pid_t childpid;
    srand(time(NULL));
    
    // sprawdzenie podanych parametrow
    if (argc < 2)
    {
        printf("Uzycie: ./client liczba_hostow\n");
        exit(0);
    }

    if (argv[1] <= 0)
    {
        printf("Zla liczba hostow\n");
        exit(0);
    }

    // pobranie z argumentu ilosci hostow
    char *c = argv[1];
    int number_of_ip = atoi(c);

    // przygotowanie listy adresow hostow na wczytanie z pliku adresow ip
    char *ip [number_of_ip]; 
    for (int j = 0; j < number_of_ip; j++)
    {
        ip[j] = NULL;
    }

    // wczytanie z pliku adresow ip
    FILE *f = fopen("ip_list","r");
    if (f == NULL)
    {
        printf("Nie mozna odczytac pliku ip_list\n");
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
    
    // sprawdzenie poprawnosci wczytanych adresow ip
    for (int j = 0; j < number_of_ip; j++)
    {
        if(inet_pton(AF_INET, ip[j], &servaddr.sin_addr)<=0)
        {
            printf("Blad konwersji do adresu IP dla %s nr: %d\n", ip[j], j);
            exit(0);
        }
    }
    // przejscie w demona

    // printf("Dziala\n");
    while(1){
        
        if((childpid = fork())==0)
        {
            // ustawienie czasu oczekiwania na odpowiedz (60 s)
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
                
            if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
            {
                printf("Blad utworzenia polaczenia\n");
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
                
                //sprawdza czy dzialanie sie wyslalo
                if (write(sockfd, "2+2 = ", sizeof("2+2 = ")) < sizeof("2+2 = ")) 
                {
                    printf("Blad wysylania\n");
                    close(sockfd);
                    exit(0);
                }

                int byte = recv(sockfd, buff, sizeof(buff), 0);
                if (byte == -1)
                {
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
        /*jezeli n jest rowne liczbie hostow program
          zawiesza dzialanie w przedziale od 10 do 15 min*/
        if(n == number_of_ip)
        {   
            n = 0;
            int min = 10; // minimalny czas oczekiwania w min
            int max = 15; // maksymalny czas oczekiwania w min
            int random = 60 * (rand() % (max - min + 1) + min);
            printf("Sleep: %d\n", random);
            sleep(random);
        }
    } 
    
    return 0;
}

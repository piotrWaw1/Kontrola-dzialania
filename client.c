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
#include <sqlite3.h>
#include <unistd.h>
#define MAXLINE 4096
#define PORT 8080
#define LISTENQ 1024

int create_db(int number_of_ip);
static int callback(void *data, int argc, char **argv, char **azColName);
void execute_database_query(int ip_id, int conn, int resp, char cwd[300]);

typedef struct{
    char address[16];
}Ip_address;

int main(int argc, char* argv[]){
    struct sockaddr_in servaddr;

    int sockfd, connfd;
    int n = 1;
    int i = 0;
    char buff[MAXLINE + 1];
    size_t len = 0;
    pid_t childpid;
    srand(time(NULL));
    char cwd [300];
    getcwd(cwd, sizeof(cwd));
    strncat(cwd, "/data.db", 9);
    // printf("%s\n",cwd);
    // sprawdzenie podanych parametrow
    if (argc < 2)
    {
        printf("Uzycie: ./client liczba_hostow tryb_pracy\n");
        exit(0);
    }

    if (argv[1] <= 0)
    {
        printf("Zla liczba hostow\n");
        exit(0);
    }
    int number_of_ip = create_db(atoi(argv[1]));
    
    // przejscie w demona
    if ((argc == 3) && (strcmp(argv[2], "1") == 0))
    {
        daemon(0, 0);
    }

    while(1){
        
        if((childpid = fork())==0)
        {
            // ustawienie czasu oczekiwania na odpowiedz (60 s)
            struct timeval tv;
            tv.tv_sec = 60;
            tv.tv_usec = 0;
            
            sqlite3 *db;
            sleep(n);
            int rc = sqlite3_open(cwd, &db);
            if(rc)
            {
                syslog(LOG_ERR, "Nie moge otworzyci pliku data.db");
            }
            char sql[100];
            Ip_address ip;

            sprintf(sql, "SELECT ip_address FROM ip WHERE id == '%d';",n);
    
            while((rc = sqlite3_exec(db, sql, callback, (void*)&ip, 0)) == 5)
            
            sqlite3_close(db);


            if((sockfd = socket(AF_INET,SOCK_STREAM,0))<0)
            {
                // printf("Blad utworzenia polaczenia\n");
                syslog(LOG_ERR, "Blad utowrzenia polaczenia");
                exit(0);
            }
                
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(PORT);
            servaddr.sin_addr.s_addr = inet_addr(ip.address);
                
            // sprawdza czy udao sie polaczyc z klientem
            if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) 
            {
                // printf("Klient nr %d nie dziala.\n", n);
                syslog(LOG_INFO, "Klient nr %d nie dziala", n);
                execute_database_query(n, 0, 0, cwd);
            }

            /*jezeli polaczenie sie uda sprawdza czy uzytkownik pracuje
            wysyla proste dzialanie 2+2 i oczekuje na odpowiedz 4*/
            else
            {
                setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                // printf("Klient nr %d dziala \n", n);
                
                //sprawdza czy dzialanie sie wyslalo
                if (write(sockfd, "2+2 = ", sizeof("2+2 = ")) < sizeof("2+2 = ")) 
                {
                    // printf("Blad wysylania\n");
                    syslog(LOG_ERR, "Blad wysylania");
                    
                }

                int byte = recv(sockfd, buff, sizeof(buff), 0);
                if (byte == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) // EGAIN i EWOULDBLOCK oznaczaja brak danych do odczytu
                    {
                        // uplyna czas oczekiwania
                        // printf("Uplyna czas oczekiwania na dane\n");
                        syslog(LOG_INFO, "Uplyna czas oczekiwania na dane. Klient nr: %d", n);
                        execute_database_query(n, 1, 0, cwd);
                    }
                    else
                    {
                        // wystapil inny blad
                        // printf("error podczas odczytu danych: %d\n", errno);
                        syslog(LOG_ERR, "error podczas odczytu danych.");
                    }
                }
                else
                {
                    // dana zostaly odebrane
                    printf("Odp: %s\n",buff);
                    if (buff[0] == '4')
                    {
                        // printf("Poprawna odpowiedz uzytkownik pracuje\n");
                        syslog(LOG_INFO, "Poprawna odpowiedz. Uzytkownik nr %d pracuje", n);
                        execute_database_query(n, 1, 1, cwd);
                    }
                    else
                    {
                        // printf("Niepoprawna odpowiedz.\n");
                        syslog(LOG_INFO, "Niepoprawna odpowiedz. Uzytkownik nr %d nie pracuje", n);
                        execute_database_query(n, 1, 0, cwd);
                    }
                }
            }
            close(sockfd);
            exit(0);
        }

        n++;
        /*jezeli n jest rowne liczbie hostow program
          zawiesza dzialanie w przedziale od 10 do 15 min*/
        if(n == number_of_ip)
        {   
            n = 1;
            int min = 10; // minimalny czas oczekiwania w min
            int max = 15; // maksymalny czas oczekiwania w min
            int random = 60 * (rand() % (max - min + 1) + min);
            printf("Sleep: %d\n", random);
            sleep(random);
        }
    } 
    
    return 0;
}

void execute_database_query(int ip_id, int conn, int resp ,char cwd[300])
{
    sqlite3 *db;

    char history [200];
    char *zErrMSG = 0;
    
    time_t t = time(NULL);
    struct tm* local_time = localtime(&t);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);

    int rc = sqlite3_open(cwd, &db);
    sprintf(history, "INSERT INTO history (id_address, date, is_working, is_responding) VALUES ('%d', '%s', '%d', '%d');", ip_id, time_str, conn, resp);
    rc = sqlite3_exec(db, history, 0, 0, 0);
    while(rc == 5)
    {
        rc = sqlite3_exec(db, history, 0, 0, 0);
    }
    
    sqlite3_close(db);

}

int create_db(int number_of_ip)
{
    struct sockaddr_in servaddr;
    sqlite3 *db;
    int rc;

    char *zErrMSG1 = 0;
    char *zErrMSG2 = 0;
    char *create_ip = "CREATE TABLE IF NOT EXISTS ip ("\
                "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"\
                "ip_address TEXT NOT NULL);";
    
    char *create_history = "CREATE TABLE IF NOT EXISTS history("\
                        "id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"\
                        "id_address INT NOT NULL,"\
                        "date DATETIME NOT NULL,"\
                        "is_working BOOL NOT NULL,"\
                        "is_responding BOOL NOT NULL,"\
                        "FOREIGN KEY (id_address) REFERENCES ip(id));";

    rc = sqlite3_open("data.db", &db);
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(0);
    } 
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    
    int rc1 = sqlite3_exec(db, create_ip, 0, 0, &zErrMSG1);
    int rc2 = sqlite3_exec(db, create_history, 0, 0, &zErrMSG2);
    
    

    if(rc1 != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMSG1);
        sqlite3_free(zErrMSG1);
    }
    if( rc2 != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMSG2);
        sqlite3_free(zErrMSG2);
    }
    
    else 
    {
      fprintf(stdout, "Table created successfully\n");
    }

    // int number_of_ip = 100;
    int i = 0;
    sqlite3_stmt *stmt;
    size_t len = 0;
    FILE *f = fopen("ip_list","r");
    char add [250]; 
    
    char *ip [number_of_ip]; 
    for (int j = 0; j < number_of_ip; j++)
    {
        ip[j] = NULL;
    }
    
    if (f == NULL)
    {
        printf("Nie mozna odczytac pliku ip_list\n");
        exit(0);
    }

    while (getline(&ip[i], &len,f) != -1)
    {
        ip[i][strlen(ip[i])-1] = '\0';

        if(i < number_of_ip)
        {
            i++;
        }
        if (i == number_of_ip)
        {
            break;
        }

    }
    
    for(int j = 0; j < number_of_ip; j++)
    {
        if(inet_pton(AF_INET, ip[j], &servaddr.sin_addr) <=0 )
        {
            printf("Blad konwersji do adresu IP dla %s nr: %d\n", ip[j], j);
            exit(0);
        }
        else
        {
            sprintf(add, "INSERT INTO ip (ip_address) SELECT '%s' WHERE NOT EXISTS (SELECT 1 FROM ip WHERE ip_address = '%s');", ip[j], ip[j]);
            rc = sqlite3_exec(db, add, 0, 0, &zErrMSG1);

            if(rc != SQLITE_OK)
            {
                fprintf(stderr, "SQL error: %s\n", zErrMSG1);
                sqlite3_free(zErrMSG1);
                sqlite3_close(db);
            }
        }
    }
    
    // Pobiera liczbe adresow ip i ja zwraca
    Ip_address number;
    rc = sqlite3_exec(db, "SELECT COUNT(*) FROM ip;", callback, (void*)&number, &zErrMSG1);

    sqlite3_close(db);
    
    return atoi(number.address);
}

static int callback(void *data, int argc, char **argv, char **azColName){

    Ip_address* ip = (Ip_address*)data;
    strcpy(ip->address, argv[0]);

    return 0;
}

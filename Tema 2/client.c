#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}
int main(int argc, char *argv[]) {

    FILE *f;
    int sockfd, is_login, n;
    char prompt[30], file_name[30];
    strcpy(file_name, "client-");
    sprintf(file_name + strlen(file_name), "%d", (int) getpid());
    strcat(file_name, ".log");

    // deschide fisierul de log
    f = fopen(file_name, "a+");
    
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer_send[BUFLEN];
    char buffer_recv[BUFLEN];

    fd_set read_fds;
    fd_set tmp_fds; 
    int fdmax;

    // verifica numarul parametrilor
    if (argc < 3) {
       fprintf(stderr, "Prea putini parametrii. Apel corect: %s server_address server_port\n", argv[0]);
       exit(0);
    }  
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);
    
    
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting"); 

    // afiseaza promptul initial
    printf("%s", "$ ");
    fflush(stdout);
    fputs("$ ", f);

    // adaug pe 0 si pe socket in multimea descriptorilor de citire
    FD_SET(0, &read_fds);
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd; 

    is_login = 0;

    while(1){

        //citesc de la tastatura
        tmp_fds = read_fds; 
        if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
            error("ERROR in select");

        // verifica daca se primeste ceva de la input
        if(FD_ISSET(0, &tmp_fds)) {

            memset(buffer_send, 0 , BUFLEN);
            fgets(buffer_send, BUFLEN-1, stdin);
            fputs(buffer_send, f);

            // daca se cere logarea si este deja cineva logat afiseaza un mesaj corespunzator
            if (strncmp(buffer_send, "login", 5) == 0 && is_login == 1) {
                fputs("-2 Sesiune deja deschisa\n", f);
                printf("-2 Sesiune deja deschisa\n");
                printf("%s", prompt);
                fflush(stdout);
                fputs(prompt, f);
            }
            // daca se cere delogarea si nu este nimeni logat afiseaza un mesaj corespunzator
            else if (strncmp(buffer_send, "logout", 6) == 0 && is_login == 0) {
                fputs("-1 Clientul nu e autentificat\n", f);
                printf("-1 Clientul nu e autentificat\n");
                printf("%s", "$ ");
                fflush(stdout);
                fputs("$ ", f);
            }
            else {
                //trimit mesaj la server
                n = send(sockfd,buffer_send,strlen(buffer_send), 0);
                if (n < 0) 
                     error("ERROR writing to socket");

                // verific daca s-a primit mesajul "quit"
                if (strncmp(buffer_send, "quit", 4) == 0) {
                    close(sockfd);
                    exit(0);
                }
            }
            
        }

        // primeste mesajele de la server
        if(FD_ISSET(sockfd, &tmp_fds)) {

            memset(buffer_recv, 0 , BUFLEN);

            n = recv(sockfd, buffer_recv, sizeof(buffer_recv), 0);
            if (n < 0) 
                error("ERROR writing to socket");

            // verifica daca serverul a cerut inchiderea clientilor
            if (strncmp(buffer_recv, "quit", 4) == 0) {
                fputs("\n", f);
                printf("\n");
                close(sockfd);
                exit(0);
            }

            // verifica daca s-a realizat logarea sau delogarea
            if (strncmp(buffer_recv, "is_login", 8) == 0) {
                if (is_login == 0)
                    is_login = 1;
                else
                    is_login = 0;

                strcpy(buffer_recv, buffer_recv + 8);
                strcpy(prompt, buffer_recv);
            }
            
            fputs(buffer_recv, f);
            printf("%s", buffer_recv);
            fflush(stdout);

            // verifica daca cineva s-a logat gresit de 3 ori la rand
            if (strncmp(buffer_recv, "-8", 2) == 0) {
                close(sockfd);
                exit(0);
            }
        }
    }

    fclose(f);
    return 0;
}
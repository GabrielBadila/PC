#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAX_CLIENTS	5 // numarul de clienti care se pot conecta la server
#define BUFLEN 256	// lungimea buffer-ului


// structura ce retine informatii pentru fiecare client conectat
struct control{
	int is_login;
	int find_user;
	int count_login;
	char prompt[30];
};


// structura ce retine informatii pentru fiecare user
struct var{
	char name[24];
	char password[24];
	char files[30][30];
	int num_files;
};


// afiseaza mesaje de eroare
void error(char *msg)
{
    perror(msg);
    exit(1);
}


// citeste utilizatorii si parolele si creeaza directoarele necesare
int read_users(struct var users[30], char users_config[30]) {

	FILE *f;
	int num_users, i;
	char temp[BUFLEN];

	if ( (f = fopen(users_config, "r")) == NULL) {
		error("Eroare la deschiderea fisisierului users_config!");
	}
	else {
		fgets(temp, BUFLEN, (FILE*)f);
		strtok(temp,"\n");
		num_users = atoi(temp); // numarul utilizatorilor

		for (i = 0; i < num_users; i++) {
			fscanf(f, "%s", users[i].name);

			// creare directoare useri
			char path[30];
			strcpy(path, users[i].name);
			int status;
	    	status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

			fgets(temp, BUFLEN, f);
			strncpy(users[i].password, temp + 1, strlen(temp) - 2);
			users[i].num_files = 0;
		}

		fclose(f);
	}

	return num_users;
}


// citeste fisierele, le verifica si le atribuie fiecarui utilizator
void read_files(struct var users[30], char shared_files[30], int num_users) {

	FILE *f;
	DIR *dir;
	struct dirent *sd;
	int num_cont, file_found, i, j;
	char temp[BUFLEN];
	char aux[30];

	if ((f = fopen(shared_files, "r")) == NULL) {
		error("Eroare la deschiderea fisisierului shared_files!");
	}
	else {
		fgets(temp, BUFLEN, f);
		strtok(temp,"\n");
		num_cont = atoi(temp); // numarul de fisiere

		for (i = 0; i < num_cont; i++) {
			fgets(temp, BUFLEN, f);
			file_found = 0;
			for (j = 0; j < num_users; j++) {
				if (strncmp(temp, users[j].name, strlen(users[j].name)) == 0) {
					memset(aux, 0 , sizeof(aux));
					strncpy(aux, temp + strlen(users[j].name) + 1, strlen(temp) - strlen(users[j].name) - 2);

					// deschide directorul unui anumit utilizator pentru verificarea fisierelor
					dir = opendir(users[j].name);

					if (dir == NULL) {
						error("Eroare la deschiderea fisisierului!");
					}

	    			while ( (sd = readdir(dir)) != NULL) {
	    				// verificarea fisierelor
	    				if (strncmp(sd->d_name, aux, strlen(aux)) == 0) {
	    					strcpy(users[j].files[users[j].num_files], aux);
							users[j].num_files++;
							file_found = 1;
							break;
	    				}
	    			}

	    			closedir(dir);
	    			break;
				}
			}

			if (file_found == 0) {
				printf("Fisierul %s nu exista!\n\n", aux);
			}
		}

		fclose(f);
	}
}


// incearca logarea unui utilizator si intoarce mesajele corespunzatoare rezultatului operatiei
void login (struct var users[30], struct control sock[30], char mes[30], int num_users, int i) {

	int j;
	char error_op[100];

	strcpy(mes, strtok (NULL, " "));
	for (j = 0; j < num_users; j++) {
		// verifica daca numele utilizatorului e corect
		if (strncmp(mes, users[j].name, strlen(mes)) == 0) {
			strcpy(mes, strtok (NULL, " "));
			sock[i].find_user = 1;
			// verifica daca parola e corecta
			if (strncmp(mes, users[j].password, strlen(users[j].password)) == 0) {
				strcpy(sock[i].prompt, users[j].name);
				strcat(sock[i].prompt, "> ");
				send(i, "is_login", strlen("is_login"), 0);
				sock[i].is_login = 1;
				sock[i].count_login = 0;
				break;
			}
			else {
				// incrementeaza numarul de incercari gresite si intoarce mesajul corespunzator
				sock[i].count_login++;
				if (sock[i].count_login == 3) {
					strcpy(error_op, "-8 Brute-force detectat\n");
					send(i, error_op, strlen(error_op), 0);
				}
				else {
					strcpy(error_op, "-3 User/parola gresita\n");
					send(i, error_op, strlen(error_op), 0);
				}
			}
		}
	}

	// incrementeaza numarul de incercari gresite si intoarce mesajul corespunzator
	if (sock[i].find_user == 0) {
		sock[i].count_login++;
		if (sock[i].count_login == 3) {
			strcpy(error_op, "-8 Brute-force detectat\n");
			send(i, error_op, strlen(error_op), 0);
		}
		else {
			strcpy(error_op, "-3 User/parola gresita\n");
			send(i, error_op, strlen(error_op), 0);
		}
	}

	sock[i].find_user = 0;
}


// intoarce lista de fisiere pentru un anumit utilizator
void getfilelist (struct var users[30], char mes[30], int num_users, int i) {

	int size, private, j, k;
	char path[30];
	char file_name[100];

	DIR *dir;
	struct dirent *sd;
	struct stat st;
	
	strcpy(mes, strtok (NULL, " "));
	for (j = 0; j < num_users; j++) {
		// verifica daca numele utilizatorului e corect
		if (strncmp(mes, users[j].name, strlen(users[j].name)) == 0) {
			// deschide directorului utilizatorului pentru a parcurge fisierele sale
			dir = opendir(users[j].name);

			if (dir == NULL) {
				error("Eroare la deschiderea fisisierului!");
			}

			while ( (sd = readdir(dir)) != NULL) {
				if (strcmp(sd->d_name, ".") != 0 && strcmp(sd->d_name, "..") != 0) {
					st.st_size = 0;
					strcpy(path, users[j].name);
					strcat(path, "/");
					strcat(path, sd->d_name);
					stat(path, &st);
					size = st.st_size;

					strcpy(file_name, sd->d_name);
					strcat(file_name, "\t");
					sprintf(file_name + strlen(file_name), "%d", size);
					strcat(file_name, " bytes\t");

					// verifica daca fisierele sunt shared sau private
					private = 1;
					for (k = 0; k < users[j].num_files; k++) {
						if (strncmp(sd->d_name, users[j].files[k], strlen(sd->d_name)) == 0) {
							private = 0;
							break;
						}
					}

					if (private == 0)
						strcat(file_name, "SHARED\n");
					else
						strcat(file_name, "PRIVATE\n");

					send(i, file_name, strlen(file_name), 0);
																				
				}
			}

			closedir(dir);
			break;
		}
	}
}


// schimba tipul unui fisier din private in shared pentru un anumit user
void share (struct var users[30], struct control sock[30], char mes[30], int num_users, int i) {

	int shared, j, k;
	int file_found = 0;
	int user_found = 0;
	char error_op[100];

	DIR *dir;
	struct dirent *sd;

	strcpy(mes, strtok (NULL, " "));
	for (j = 0; j < num_users; j++) {
		// cauta utilizatorul in lista de utilizatori
		if (strncmp(users[j].name, sock[i].prompt, strlen(sock[i].prompt) - 2) == 0) {

			user_found = 1;
			// deschide directorului utilizatorului pentru a parcurge fisierele sale
			dir = opendir(users[j].name);

			if (dir == NULL) {
				error("Eroare la deschiderea fisisierului!");
			}

			shared = 0;
			while ( (sd = readdir(dir)) != NULL) {
				// verifica daca fisierul e deja partajat
				if (strncmp(mes, sd->d_name, strlen(sd->d_name)) == 0) {

					file_found = 1;
					for (k = 0; k < users[j].num_files; k++) {
						if (strncmp(mes, users[j].files[k], strlen(users[j].files[k])) == 0) {
							strcpy(error_op, "-6 Fisierul este deja partajat\n");
							send(i, error_op, strlen(error_op), 0);
							shared = 1;
							break;
						}
					}

					// daca fisierul nu a fost partajat atunci il partajeaza
					if (shared == 0) {
						strcpy(users[j].files[users[j].num_files], sd->d_name);
						users[j].num_files++;

						strcpy(error_op, "200 Fisierul ");
						strcat(error_op, sd->d_name);
						strcat(error_op, " a fost partajat\n");
						send(i, error_op, strlen(error_op), 0);
					}

					break;
				}
			}
			break;
		}
	}

	// verifica daca fisierul exista
	if (user_found == 0 || file_found == 0) {
		strcpy(error_op, "-4 Fisier inexistent\n");
		send(i, error_op, strlen(error_op), 0);
	}
}


// schimba tipul unui fisier din shared in private pentru un anumit utilizator
void unshare (struct var users[30], struct control sock[30], char mes[30], int num_users, int i) {

	int private, j, k, l;
	int file_found = 0;
	int user_found = 0;
	char error_op[100];

	DIR *dir;
	struct dirent *sd;

	strcpy(mes, strtok (NULL, " "));
	for (j = 0; j < num_users; j++) {
		// cauta utilizatorul in lista de utilizatori
		if (strncmp(users[j].name, sock[i].prompt, strlen(sock[i].prompt) - 2) == 0) {

			user_found = 1;
			// deschide directorului utilizatorului pentru a parcurge fisierele sale
			dir = opendir(users[j].name);

			if (dir == NULL) {
				error("Eroare la deschiderea fisisierului!");
			}

			private = 0;
			while ( (sd = readdir(dir)) != NULL) {
				// verifica daca fisierul e partajat
				if (strncmp(mes, sd->d_name, strlen(sd->d_name)) == 0) {

					file_found = 1;
					for (k = 0; k < users[j].num_files; k++) {
						if (strncmp(mes, users[j].files[k], strlen(users[j].files[k])) == 0) {

							if (k == users[j].num_files - 1) {
								strcpy(users[j].files[k], "");
								users[j].num_files--;
							}
							else {
								for (l = k; l < users[j].num_files - 1; l++)
									strcpy(users[j].files[l], users[j].files[l + 1]);
								strcpy(users[j].files[users[j].num_files - 1], "");
								users[j].num_files--;
							}

							strcpy(error_op, "200 Fisierul ");
							strcat(error_op, sd->d_name);
							strcat(error_op, " a fost setat ca PRIVATE\n");
							send(i, error_op, strlen(error_op), 0);
							private = 1;
							break;
						}
					}

					// verifica daca fisier e deja private
					if (private == 0) {
						strcpy(error_op, "-7 Fisier deja privat\n");
						send(i, error_op, strlen(error_op), 0);
					}

					break;
				}
			}
			break;
		}
	}

	// verifica daca fisierul exista
	if (user_found == 0 || file_found == 0) {
		strcpy(error_op, "-4 Fisier inexistent\n");
		send(i, error_op, strlen(error_op), 0);
	}
}


// sterge un fisier pentru un anumit utilizator
void delete (struct var users[30], struct control sock[30], char mes[30], int num_users, int i) {

	int j, k, l;
	int file_found = 0;
	int user_found = 0;
	char file_name[30];
	char path[30];
	char error_op[100];

	DIR *dir;
	struct dirent *sd;
	struct stat st;

	strcpy(mes, strtok (NULL, " "));
	for (j = 0; j < num_users; j++) {
		// cauta utilizatorul in lista de utilizatori
		if (strncmp(users[j].name, sock[i].prompt, strlen(sock[i].prompt) - 2) == 0) {

			user_found = 1;
			// deschide directorului utilizatorului pentru a parcurge fisierele sale
			dir = opendir(users[j].name);

			if (dir == NULL) {
				error("Eroare la deschiderea fisisierului!");
			}

			while ( (sd = readdir(dir)) != NULL) {
				// cauta fisierul
				if (strncmp(mes, sd->d_name, strlen(sd->d_name)) == 0) {

					file_found = 1;
					strcpy(path, users[j].name);
					strcat(path, "/");
					strcat(path, sd->d_name);
					strcpy(file_name, sd->d_name);

					// stergerea fisierului
					if (unlink(path) == 0) {
						for (k = 0; k < users[j].num_files; k++) {
							if (strncmp(mes, users[j].files[k], strlen(users[j].files[k])) == 0) {

								if (k == users[j].num_files - 1) {
									strcpy(users[j].files[k], "");
									users[j].num_files--;
								}
								else {
									for (l = k; l < users[j].num_files - 1; l++)
										strcpy(users[j].files[l], users[j].files[l + 1]);
									strcpy(users[j].files[users[j].num_files - 1], "");
									users[j].num_files--;
								}
								break;
							}
						}


						strcpy(error_op, "200 Fisierul ");
						strcat(error_op, file_name);
						strcat(error_op, " a fost sters\n");
						send(i, error_op, strlen(error_op), 0);

					}
					else
						error("Fisierul nu a putut fi sters!");

					break;
				}
			}
			break;
		}
	}

	// verifica daca fisierul exista
	if (user_found == 0 || file_found == 0) {
		strcpy(error_op, "-4 Fisier inexistent\n");
		send(i, error_op, strlen(error_op), 0);
	}
}


int main(int argc, char *argv[]) {	

	struct var users[30];
	struct control sock[30];
	struct sockaddr_in serv_addr, cli_addr;
	int num_users, num_cont, sockfd, newsockfd, portno, clilen, i, j, k, n;
	char temp[BUFLEN], buffer[BUFLEN];

	// initializare variabile de control
	for (i = 0; i < 30; i++) {
		sock[i].is_login = 0;
		sock[i].find_user = 0;
		sock[i].count_login = 0;
	}


    fd_set read_fds;	//multimea de citire folosita in select()
    fd_set tmp_fds;	//multime folosita temporar 
    int fdmax;		//valoare maxima file descriptor din multimea read_fds

    // verifica numarul parametrilor
    if (argc < 4) {
        fprintf(stderr, "Prea putini parametrii. Apel corect: %s port users_config shared_files\n", argv[0]);
        exit(1);
    }

    // citirea utilizatorilor, parolelor si a fisierlor corespunzatoare
    char users_config[30];
    strcpy(users_config, argv[2]);
    char shared_files[30];
    strcpy(shared_files, argv[3]);

	num_users = read_users(users, users_config);
	read_files(users, shared_files, num_users);

    // golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
     
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
		error("ERROR opening socket");
     
    portno = atoi(argv[1]); // portul

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
    serv_addr.sin_port = htons(portno);
     
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
        error("ERROR on binding");
     
    listen(sockfd, MAX_CLIENTS);

    // adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    FD_SET(0, &read_fds);

    // aici are loc verificarea fiecarei comenzi
	while (1) {
		tmp_fds = read_fds; 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) 
			error("ERROR in select");
	
		// verifica primirea mesajului "quit" si realizeaza inchiderea serverului si a clientilor
		if(FD_ISSET(0, &tmp_fds)) {
        	memset(buffer, 0 , BUFLEN);
        	fgets(buffer, BUFLEN-1, stdin);
        	if (strncmp(buffer, "quit", 4) == 0) {
                for(i = 4; i <= fdmax; i++) {
					if (FD_ISSET(i, &read_fds)) {
	                	send(i, "quit", strlen("quit"), 0);
		                if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
							if (n == 0) {
								//conexiunea s-a inchis
								printf("server: socket %d hung up\n", i);
							} else {
								error("ERROR in recv");
							}
							close(i); 
							FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care l-am inchis
						}
					}
                }
                close(sockfd);
                exit(0);
            }
        }


		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					} 
					else {
						// adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
					}

					printf("Noua conexiune de la %s, port %d, socket_client %d\n\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}
					
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					// actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("server: socket %d hung up\n", i);
						} else {
							error("ERROR in recv");
						}
						close(i); 
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care l-am inchis
					} 
					
					else { //recv intoarce >0
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);

						char mes[30];
						strcpy(mes, strtok(buffer, " "));

						// verificarea fiecarei comenzi
						// daca ce s-a primit nu e o comanda afisez in consola serverului un mesaj corespunzator
						if (strcmp(buffer, "\n") == 0)
							printf("%s\n", "Mesaj gol!\n");
						else if (strcmp(mes, "login") == 0) {
							login(users, sock, mes, num_users, i);
						}
						// delogheaza un utilizator
						else if (strncmp(buffer, "logout", 6) == 0) {
								send(i, "is_login", strlen("is_login"), 0);
								sock[i].is_login = 0;
						}
						// intoarce lista de utilizatori continuta de server
						else if (strncmp(buffer, "getuserlist", 11) == 0 && sock[i].is_login) {
							for (j = 0; j < num_users; j++) {
								strcpy(mes, users[j].name);
								strcat(mes, "\n");
								send(i, mes, strlen(mes), 0);
							}
						}
						else if (strcmp(mes, "getfilelist") == 0 && sock[i].is_login) {
							getfilelist(users, mes, num_users, i);
						}
						else if (strcmp(mes, "share") == 0 && sock[i].is_login) {
							share(users, sock, mes, num_users, i);
						}
						else if (strcmp(mes, "unshare") == 0 && sock[i].is_login) {
							unshare(users, sock, mes, num_users, i);
						}
						else if (strcmp(mes, "delete") == 0 && sock[i].is_login) {
							delete(users, sock, mes, num_users, i);
						}
						// inchide si elimina din multimea descriptorilor de citire clientii ce au cerut inchiderea
						else if (strncmp(buffer, "quit", 4) == 0) {
							memset(buffer, 0, BUFLEN);
							if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
								if (n == 0) {
									//conexiunea s-a inchis
									printf("server: socket %d hung up\n", i);
								} else {
									error("ERROR in recv");
								}
								close(i); 
								FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care l-am inchis
							} 
						}
						else 
							printf("%s\n", "Mesaj oarecare!\n");

						// intorc promptul "$" sau promptul utilizatorului, dupa caz
						if (sock[i].count_login < 3) {
							if (sock[i].is_login == 0)
								send(i, "$ ", strlen("$ "), 0);
							else
								send(i, sock[i].prompt, strlen(sock[i].prompt), 0);
						}
					}
				} 
			}
		}
     }

    close(sockfd);

	return 0;
}
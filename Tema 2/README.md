# Sisteme de Backup si Partajare a Fisierelor (Romanian language)


	Protocoale de Comunicatie
	
	Tema 2 - Sisteme de Backup si Partajare a Fisierelor
	
	Badila Gabriel Alin
	323 CA
	
	
	============================================================================
	
	IMPLEMENTARE
	
	Aceasta tema realizeaza o conexiune client-server, respectiv clienti-server, 
	utilizand protocolul TCP ce foloseste apelul select() pentru multiplexare.
	Apelul select() primeste ca argumente pointeri catre trei multimi de 
	descriptori (citire, scriere si exceptii), eu am lucrat doar cu multimea de 
	citire asa ca pe ceilalti i-am lasat la NULL. Initial in multimea de citire 
	a serverului se afla doar descriptorul pentru socketul inactiv, dar pe 
	masura ce se conecteaza clientii, se vor adauga si descriptori pentru 
	socketii activi.

	Voi lua comenzile din cadrul temei pe rand si voi oferi cateva informatii 
	asupra implentarii mele:

	login <nume_utilizator> <parola>:
	
		In momentul cand un client introduce aceasta comanda se verifica intai, 
		in cadrul clientului, daca este vreun utilizator conectat in momentul 
		acela si se intoarce un mesaj corespunzator. Daca niciun utilizator nu 
		este conectat comanda este trimisa catre server, unde este verificata 
		corectitudinea numelui si a parolei si se intoarce dupa caz: mesaj de 
		eroare (unul sau ambele argumente sunt gresite) sau un nou prompt, cu 
		numele utilizatorului logat (logarea a reusit). In cazul in care au loc 
		trei incercari de logare nereusite, consecutive, este intors la client 
		un alt mesaj de eroare, iar clientul isi incheie executia.

	logout:
	
		Se verifica intai in cadrul serverului ca un utilizator sa fie logat, 
		altfel se afiseaza un mesaj de eroare. Daca cineva e logat mesajul este 
		trimis la server unde se realizeaza delogarea, iar promptul ce inainte 
		era format din numele unui utilizator se transforma in promptul de baza:
		"$".

	getuserlist:
		
		Mesajul este transmis direct catre server unde se parcurge lista de 
		utilizatori, creata la executia serverului prin citirea utilizatorilor 
		din fisierul users_config, si se intorc toti utilizatorii. Aceasta 
		comanda functioneaza doar atunci cand un utilizator este logat.
		
	getfilelist <nume_utilizator>:
	
		Aceasta comanda poate fi folosita doar atunci cand un utilizator este 
		logat. Mesajul este transmis la server, iar aici se deschide directorul 
		utilizatorului specificat si se intoarce fiecare fisier precizand 
		dimensiunea si tipul sau (shared - se afla in fisierul shared_files, 
		private).
		
	share <nume_fisier>:
	
		Aceasta comanda poate fi folosita doar atunci cand un utilizator este 
		logat si doar pe fisierele sale. Mesajul este trimis la server, iar aici 
		se verifica daca fisierul exista (se afiseaza un anumit mesaj de eroare 
		daca nu exista), daca este deja partajat (alt mesaj de eroare). Daca 
		fisierul exista, dar nu este partajat se realizeaza partajarea sa si se 
		intoarce un mesajul ce confirma reusita comenzii.
		
	unshare <nume_fisier>:

		Realizeaza operatia inversa a comenzii share, deci implementarea este 
		similara.
		
	delete <nume_fisier>:
	
		Aceasta comanda se poate utiliza doar atunci cand utilizator este logat 
		si doar pentru fisierele sale. Mesajul este trimis la server, iar aici 
		daca fisierul in cauza exista are loc stergerea lui si intoarcerea unui 
		mesaj de reusita, altfel va fi intors un mesaj de eroare.
		
	quit (client):

		Clientul trimite acest mesaj catre server si se inchide. Serverul 
		primeste mesajul, verifica daca mai vin mesaje de la client, iar apoi 
		inchide conexiunea si il elimina din lista de descriptori.
		
	quit (server):
	
		Serverul trimite un mesaj la toti clienti, prin care le cere sa se 
		inchida. Apoi verifica daca mai primeste mesaje de la ei, inchide 
		conexiunea cu ei si ii elimina din lista de descriptori. In cele din urma 
		are loc inchiderea socket-ului, asadar inchiderea instantei server.

	============================================================================
	
	CONCLUZII
	
	In aceasta tema sunt implementate toate functiile, exceptand "upload" si 
	"download". Tema functioneaza pe un numar maxim de 5 clienti, dar acest 
	lucru se poate schimba foarte usor, in cadrul serverului, prin modificarea 
	variabilei "MAX_CLIENTS". La parcurgerea fisierelor, de pe fiecare linie se 
	citeste un numar de 256 de caractere (BUFLEN), acest lucru de asemenea poate 
	fi modificat in cadrul serverului.

	In concluzie, chiar daca nu am implementat o functionalitate importanta a 
	temei, partajarea fisierelor (upload si download), consider ca am inteles 
	destul de bine cum se lucreaza cu socketi, utilizand protocolul TCP, dar mai 
	ales cum functioneaza apelul select().

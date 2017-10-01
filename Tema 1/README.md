# Let's play a game (Romanian language)


	Protocoale de Comunicatie
	
	Tema 1 - Let's play a game
	
	Badila Gabriel Alin
	323 CA
	
	
	============================================================================
	
	IMPLEMENTARE
	
	Dupa parerea mea aceasta tema este bazata in mare parte pe hard-codare 
	(primesti mesajul de la client, vezi ce vrea si te conformezi), dar am 
	inteles ca scopul temei nu este neaparat implementarea eficienta, ci crearea 
	unui canal de comunicatie intre server si client, prin care informatia sa 
	circule corect.

	Voi lua taskurile pe rand si voi oferi cateva informatii asupra implentarii 
	mele:

	Taskul 1:
		Aici este realizat schimbul de informatie in cel mai simplu mod, primesc 
		mesajul de la client, vad ce doreste, iar apoi trimit mesajul meu.
		La cautarea binara, dupa ce trimit un numar astept mesajul clientului si 
		verific de ce tip e (smaller, bigger, success), pentru a realiza 
		modificarile corespunzatoare (micsorez limita superioara, maresc limita 
		inferioara sau in caz de success, numarul a fost gasit, parasesc bucla 
		while).

	Taskul 2:
		Comunicarea dintre server si client se face asemanator cu cea de la 
		task-ul 1, singura diferenta o face faptul ca dupa fiecare mesaj primit, 
		trimit clientului un "ACK" (acknowledge), pentru a sti ca am primit 
		mesajul sau. La fel si invers, de fiecare data cand trimit un mesaj 
		astept un "ACK" de la client, pentru a sti ca si el, la randul lui, a 
		primit mesajul meu.

	Taskul 3:
		Pentru acest task functiile de send si receive au o alta structura, 
		de aceasta data mesajele pot fi eronate.
		Pentru verificarea erorilor folosesc un bit de paritate. Am doua 
		functii care ma ajuta sa stabilesc bitul de paritate al unui mesaj: 
		"getBit" (ce returneaza bitul de paritate al unui caracter) si 
		"getParity" (ce apeleaza getBit si returneaza bitul de paritate al 
		intregului mesaj).

		In functia "send2" se realizeaza urmatoarele operatii:
		- pun bitul de paritate al mesajului in r.payload[0], apoi pun mesajul
		- trimit r.payload
		- primesc "ACK" (mesaj corect, s-a terminat etapa de trimitere) sau 
		"NACK" (mesaj incorect, repet etapele de mai sus pana primesc "ACK").
		Verificarea mesajelor de confirmare se face pe baza lungimii lor, pentru 
		ca si ele pot fi eronate si asta e singura cale.

		Pentru primirea mesajelor de la client am 2 functii (receive2 si 
		receiveB). In principiu ele fac cam acelasi lucru, singura diferenta 
		fiind faptul ca in "receive2", la sfarsit, este trimis si un mesaj "ACK",
		si poate faptul ca "recieveB" intoarce ceva.
		Pentru primirea mesajelor pasii sunt urmatorii:
		- primesc mesajul de la client
		- verifica ca r.payload[0] (paritatea) sa fie egala cu paritatea 
		calculata de mine pe mesajul primit, daca nu sunt se repeta procesul de 
		mai sus
		- in momentul cand sunt egale, mesajul a fost primit corect, se trimite 
		"ACK"
		Functia "receiveB" este folosita doar in cadrul cautarii binare.

	============================================================================
	
	CONCLUZII
	
	Tema a fost ambigua, nu neaparat enuntul, ci implementarea ei, asa-zisele 
	scapari, precum punerea bitului de paritate in lungimea mesajului 
	(r.len = strlen(r.payload) + 2) au consumat multora dintre noi destul timp, 
	si nu au fost chiar asa usor de depistat. Chiar si checker-ul local a avut
	bug-urile lui: dupa terminarea primului task, la rularea checker-ului am 
	observat ca am implementat deja o tema de 100 pct.

	In concluzie, dupa explicatiile de pe forum am reusit sa implementez primele
	3 taskuri, de al 4-lea nu am mai avut timp, si consideram ca a fost un 
	exemplu destul de bun pentru a intelege traseul informatiei intr-o retea, 
	intre 2 puncte.
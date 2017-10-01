#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

// returneaza bitul de paritate pentru un caracter
int getBit(char c) { 
	unsigned char mask = 1<<7; 
	int i, result = 0; 
	for(i = 0; i < 8; i++) { 
		result ^= ((c&mask)!=0); 
		mask>>=1; 
	} 
	return result; 
} 

// returneaza bitul de paritate pentru intregul mesaj
int getParity (char *c, int len) { 
	int sum = 0, i = 0; 
	while(i < len) { 
		sum ^= getBit(c[i]); 
		i++; 
	} 
	return sum; 
}

// realizeaza trimiterea unui mesaj
void send (char* message) {
	msg r;
	int res;
	
	sprintf(r.payload, "%s", message);
	r.len = strlen(r.payload) + 1;

	res = send_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Send ACK error. Exiting.\n");
		return;
	}
}

// realizeaza primirea unui mesaj
char* receive () {
	msg r;
	int res;
	
	res = recv_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Receive error. Exiting.\n");
		return 0;
	}
	printf("%s\n", r.payload);
	
	char* msg = r.payload;
	return msg;
}

// realizeaza trimiterea unui mesaj si setarea bitului de paritate, pentru 
// verificarea ulterioara facuta de catre client
void send2 (char* message) {
	
	msg r;
	int res;

	r.payload[0] = getParity(message, strlen(message));	
	sprintf(r.payload + 1, "%s", message);
	r.len = strlen(message) + 2;

	res = send_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Send ACK error. Exiting.\n");
		return;
	}
		
	res = recv_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Receive error. Exiting.\n");
		return;
	}
	printf("S1: %s\n", r.payload);

	// cat timp primesc "NACK" trimit din nou mesajul
	while (r.len == 5) {
		r.payload[0] = getParity(message, strlen(message));	
		sprintf(r.payload + 1, "%s", message);
		r.len = strlen(r.payload) + 2;
	
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return;
		}

		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return;
		}
		printf("S-while: %s\n", r.payload);
	}
	
}

// realizeaza primirea unui mesaj, verificarea lui si trimiterea dupa caz a 
// unuia din mesajele: "ACK" si "NACK"
void receive2() {

	msg r;
	int res;
	
	res = recv_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Receive error. Exiting.\n");
		return;
	}
	printf("R1: %s\n", r.payload + 1);
	
	// daca mesajul nu e corect trimit "NACK" si astept primirea altuia
	while (getParity(r.payload + 1, r.len) != r.payload[0]) {
	
		send("NACK");
		
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return;
		}
		printf("R-while: %s\n", r.payload + 1);
	}
	
	// mesajul primit e corect, trimit "ACK"
	send("ACK");

}

// realizeaza primirea mesajului si verificarea lui
// functie folosita doar in cautarea binara de la taskul 3, trimiterea de "ACK"
// se realizeaza in cautarea binara din main
char* receiveB() {

	msg r;
	int res;
	
	res = recv_message(&r);
	if (res < 0) {
		perror("[RECEIVER] Receive error. Exiting.\n");
	}
	printf("RB: %s\n", r.payload + 1);

	// daca mesajul nu e corect trimit "NACK" si astept primirea altuia
	while (getParity(r.payload + 1, r.len) != r.payload[0]) {
	
		send("NACK");

		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
		}
		printf("RB-while: %s\n", r.payload + 1);
	}
	
	char* msg = r.payload + 1;
	printf("MSG: %s\n", msg);
	
	return msg;

}

int main(int argc, char *argv[])
{
	int lower = 0, upper = 999, mid = 0;
	char temp[30];
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	

	// TASKUL 1

	if (argc == 1) {
		receive();
		send("Hello");
		receive();
		receive();
		receive();
		send("YEY");
		send("OK");
		receive();
		
		// cautarea binara pentru aflarea numarului generat random de client
		while (lower <= upper) {
			mid = lower + (upper - lower)/2;
			sprintf(temp, "%d", mid);
			
			send(temp);
			
			sprintf(temp, "%s", receive());
			
			if (strncmp(temp, "smaller", 7) == 0)
				upper = mid - 1;
			else if (strncmp(temp, "bigger", 6) == 0)
				lower = mid + 1;
			else
				break;
		}
		
		receive();
		send("exit");
	}


	// TASKUL 2

	else if (argc == 2 && strncmp(argv[1], "ack", 3) == 0) {
		receive();
		send("ACK");
		send("Hello");
		receive();
		receive();
		send("ACK");
		receive();
		send("ACK");
		receive();
		send("ACK");
		send("YEY");
		receive();
		send("OK");
		receive();
		receive();
		send("ACK");

		// cautarea binara pentru aflarea numarului generat random de client
		while (lower <= upper) {
			mid = lower + (upper - lower)/2;
			sprintf(temp, "%d", mid);
			
			send(temp);
			receive();
			
			sprintf(temp, "%s", receive());
			
			if (strncmp(temp, "smaller", 7) == 0)
				upper = mid - 1;
			else if (strncmp(temp, "bigger", 6) == 0)
				lower = mid + 1;
			else
				break;

			send("ACK");
		}

		send("ACK");
		receive();
		send("ACK");
		send("exit");
	}


	// TASKUL 3

	else if (argc == 2 && strncmp(argv[1], "parity", 6) == 0) {
		receive2();
		send2("Hello");
		receive2();
		receive2();
		receive2();
		send2("YEY");
		send2("OK");
		receive2();

		// cautarea binara pentru aflarea numarului generat random de client
		while (lower <= upper) {
			mid = lower + (upper - lower)/2;

			sprintf(temp, "%d", mid);
			send2(temp);

			sprintf(temp, "%s", receiveB());
			
			if (strncmp(temp, "smaller", 7) == 0)
				upper = mid - 1;
			else if (strncmp(temp, "bigger", 6) == 0)
				lower = mid + 1;
			else
				break;
				
			send("ACK");
		}

		send("ACK");
		
		receive2();
		send2("exit");

	}


	// TASKUL 4

	else if (argc == 2 && strcmp(argv[1], "hamming") == 0) {
		receive();
		send("test");
	}

	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}

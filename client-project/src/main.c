#if defined WIN32
#include <winsock2.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include "protocol.h"
//////////////CLIENT
void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(int argc, char *argv[]) {
    if (argc < 3 || strcmp(argv[1], "-r") != 0) {
        printf("Usage: %s -r \"type city\"\n", argv[0]);
        printf("Esempio: %s -r \"t bari\"\n", argv[0]);
        return -1;
    }
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	// create client socket
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	// set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP del server
	sad.sin_port = htons(PROTO_PORT); // Server port

	// connection
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("Failed to connect.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	weather_request_t wr;
	// Parsing corretto per gestire "p Reggio Calabria"
	for (int i = 1; i < argc; i++) {
	    if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
	        char *request_str = argv[++i];

	        // Prendi il primo carattere come type
	        wr.type=request_str[0];

	        // Tutto il resto della stringa è la città
	        // Salta il type e lo spazio (se presente)

	        if (strlen(request_str) >= 2 && request_str[1] == ' ') {
	            strcpy(wr.city, &request_str[2]);
	        } else {
	            // Se non c'è spazio, prendi tutto dopo il primo carattere
	            strcpy(wr.city, &request_str[1]);
	        }

	    }
	}

	// send data to server
	if (send(c_socket, (char *)&wr, sizeof(weather_request_t), 0) != sizeof(weather_request_t)) {
		errorhandler("send() sent a different number of bytes than expected");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	weather_response_t risposta;

	// Ricevi la risposta dal server
	if ((recv(c_socket, (char *)&risposta, sizeof(weather_response_t), 0)) <= 0) {
	    errorhandler("recv() failed or connection closed prematurely");
	    closesocket(c_socket);
	    clearwinsock();
	    return -1;
	}
	printf("Ricevuto risultato dal server ip %s. ", inet_ntoa(sad.sin_addr));


	switch(risposta.status){
	case 0: printf("%s :",wr.city);
			switch (risposta.type){
			case 't': printf("Temperatura = ");
					printf("%.1f°C", risposta.value);
					break;
			case 'h': printf("Umidità = ");
			printf("%.1f%%", risposta.value);
					break;
			case 'w' : printf("Vento = ");
			printf("%.1f km/h", risposta.value);
					break;
			case 'p':  printf("Pressione =");
			printf("%.1f hPa", risposta.value);
					break;
			}

	break;
	case 1:printf("Città non disponibile");
	break;
	case 2:printf("Richiesta non valida");
	break;
	}
	printf("\n");
	closesocket(c_socket);
	clearwinsock();
	return 0;
} // main end

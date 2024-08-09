#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_CON 20
#define HTML_FILE "index.html"

const char *response =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n\r\n";

int main(int argc, char *argv[]) {
	
	unsigned int port;
	int ssock, csock;
	struct sockaddr_in server, client;
	struct stat st;
	socklen_t client_len = sizeof(client);
	FILE *f;
	char *buf;

	if (argc < 2) {
		puts("Provide a port number.");
		return -1;
	}

	port = strtol(argv[1], NULL, 10);

	if (!port) {
		puts("Choose a non zero port number.");
		return -1;
	}

	f = fopen(HTML_FILE,"r");
	if (!f) {
		perror("Failed to open file.");
		return -1;
	}

	fstat(fileno(f), &st);
	buf = malloc(st.st_size + 1);
	if (!buf) {
		perror("Memory allocation failed.");
		return -1;
	}

	memset(buf, '\0', st.st_size + 1);
	fread(buf, st.st_size, 1, f);

	ssock = socket(AF_INET, SOCK_STREAM, 0);

	if (ssock == -1) {
		perror("Error creating a socket.");
		return -1;
	}


	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(ssock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Bind failed.");
		return -1;
	}

	if (listen(ssock, MAX_CON) < 0) {
		perror("Listen failed.");
		return -1;
	}

	while (1) {
		csock = accept(ssock, (struct sockaddr *)&client, &client_len);
		if (csock < 0) {
			perror("Accept failed.");
			break;
		}

		send(csock, response, strlen(response), 0);
		send(csock, buf, st.st_size + 1, 0);
		send(csock, "\r\n",strlen("\r\n"),0);

		close(csock);
	}

	free(buf);
	fclose(f);

	return 0;
}


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
	int ssock = -1, csock = -1;
	struct sockaddr_in server, client;
	struct stat st;
	socklen_t client_len = sizeof(client);
	FILE *f;
	char *buf;
	size_t read_size;

	if (argc < 2) {
		puts("Provide a port number");
		return -1;
	}

	port = strtol(argv[1], NULL, 10);

	if (!port) {
		puts("Choose a non zero port number");
		return -1;
	}

	f = fopen(HTML_FILE,"r");
	if (!f) {
		perror("Failed to open file");
		goto cleanup;
	}

	if (fstat(fileno(f), &st) == -1){
		perror("Failed to get files stats");
		goto cleanup;
	};

	buf = calloc(st.st_size + 1, 1);
	if (!buf) {
		perror("Memory allocation failed");
		goto cleanup;
	}

	read_size = fread(buf, 1, st.st_size, f);
	if (read_size != st.st_size) {
		perror("Failed to read entire file");
		goto cleanup;
	}

	ssock = socket(AF_INET, SOCK_STREAM, 0);
	if (ssock == -1) {
		perror("Error creating a socket");
		goto cleanup;
	}


	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(ssock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Bind failed");
		goto cleanup;
	}

	if (listen(ssock, MAX_CON) < 0) {
		perror("Listen failed");
		goto cleanup;
	}

	while (1) {
		csock = accept(ssock, (struct sockaddr *)&client, &client_len);
		if (csock < 0) {
			perror("Accept failed");
			break;
		}

		if (send(csock, response, strlen(response), 0) < 0) {
			perror("Send failed");
			close(csock);
			continue;
		}

		if (send(csock, buf, st.st_size, 0) < 0) {
			perror("Send failed");
			close(csock);
			continue;
		};

		close(csock);
	}

cleanup:
	if (csock != -1) close(csock);
	if (ssock != -1) close(ssock);
	if (buf) free(buf);
	if (f) fclose(f);

	return 0;
}


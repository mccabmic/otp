/*
 * client.c
 */
#define __GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXDATASIZE 1000

char* read_file(char* filename){
	if (access(filename, F_OK) == -1){
		fprintf(stderr, "File does not exist: %s\n", filename);
		exit(2);
	}

	FILE *fp = fopen(filename, "r");
	if (fp < 0){
		fprintf(stderr, "Failure to open file %s\n", filename);
		exit(2);
	}

	size_t buffer_size;
	char* file_contents = NULL;
	size_t numCharacters = getdelim(&file_contents, &buffer_size, '\0', fp);
	fclose(fp);
	if (numCharacters != -1){
		return file_contents;
	}
	else {
		free (file_contents);
		return 0;
	}
}

int main(int argc, char *argv[]){
	// verify arguments
	if (argc != 4){
		printf("%s: usage - %s plaintext key port\n", __FILE__, __FILE__);
		exit(2);
	}
	
	// set port
	char* PORT = argv[3];
	
	// read contents of plaintext and verify no bad chars
	
	char* plaintext = read_file(argv[1]);
	char* key = read_file(argv[2]);
	
	printf("plaintext l: %d key l: %d\n", strlen(plaintext), strlen(key));	
	free(plaintext);
	free(key);

	return 0;

	// read contents of mykey and verify no bad chars
	// compare length of plaintext to mykey and return if bad
	

	struct addrinfo hints, *servinfo, *traverse;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // change to AF_UNSPEC for IPv6 comp
	hints.ai_socktype = SOCK_STREAM;
	int rv;
	

	if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "%s: getaddrinfo error: %s\n", __FILE__, gai_strerror(rv));
		return 2;
	}

	int connect_socket;
	char s[INET6_ADDRSTRLEN];

	for (traverse = servinfo; traverse != NULL; traverse=traverse->ai_next){
		if ((connect_socket = socket(traverse->ai_family, traverse->ai_socktype,
				traverse->ai_protocol)) == -1){
			fprintf(stderr, "%s: socket error\n", __FILE__);
			continue;
		}
		
		inet_ntop(traverse->ai_family, traverse->ai_addr, s, sizeof(s));
		printf("%s: attempting connection to: %s\n", __FILE__, s);
		if (connect(connect_socket, traverse->ai_addr, traverse->ai_addrlen) == -1){
			close(connect_socket);
			fprintf(stderr, "%s: connect error\n", __FILE__);
			continue;
		}
		break;
	}

	if (traverse == NULL){
		fprintf(stderr, "%s: could not connect\n", __FILE__);
		exit(2);
	}

	freeaddrinfo(servinfo);
	char buf[MAXDATASIZE];
	int numbytes;
	
	// verify identity to server
	send(connect_socket, "e#PLAINTEXT%KEYTEXTKEYTEXT@", 28, 0);
	
	if ((numbytes = recv(connect_socket, buf, MAXDATASIZE-1,0)) == -1){
		fprintf(stderr, "%s: recv error\n", __FILE__);
		exit(2);
	}
	
	// receive confirmation from server
	buf[numbytes] = '\0';
	printf("%s received: '%s'\n", __FILE__, buf);
	
	// if verified, time to do work
	
	// otherwise, exit with error
	
	
	close(connect_socket);
	return 0;
}

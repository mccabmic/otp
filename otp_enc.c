/*
 * client.c
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXDATASIZE 1000

bool validate(char* filename){
	size_t str_len = strlen(filename);
	int c;
	for (c = 0; c < str_len; c++){
		if (!isalpha(filename[c]) && !isspace(filename[c])){
			return false;
		}
	}
	return true;
}

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
		file_contents[numCharacters-1]='\0';
		return file_contents;
	}
	else {
		free (file_contents);
		return 0;
	}
}

int sendall(int socket, char *buf, int *len){
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total <* len){
		n = send(socket, buf+total, bytesleft, 0);
		if (n == -1){break;}
		total += n;
		bytesleft -= n;
	}
	*len = total;
	return n==-1?-1:0;
}

int main(int argc, char *argv[]){
	// verify arguments
	if (argc != 4){
		printf("%s: usage - %s plaintext key port\n", __FILE__, __FILE__);
		exit(1);
	}
	
	// set port
	char* PORT = argv[3];
	
	// read contents of plaintext and verify no bad chars
	char* plaintext = read_file(argv[1]);
	char* key = read_file(argv[2]);
	
	int pt_len, key_len;
	pt_len = strlen(plaintext);
	key_len = strlen(key);
	if (pt_len != key_len){
		fprintf(stderr, "%s: plaintext len != key len\n", __FILE__);
		exit(1);
	}
	
	if (!validate(plaintext)){
		fprintf(stderr, "%s: input contains bad characters\n", __FILE__);
		exit(1);
	};

	if (!validate(key)){
		fprintf(stderr, "%s: input contains bad characters\n", __FILE__);
		exit(1);
	}
		
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
		free(plaintext);
		free(key);
		exit(2);
	}

	freeaddrinfo(servinfo);
	
	// verify identity to server
	send(connect_socket, "e", 1, 0);
	// receive confirmation from server
	char auth = '\0';
	recv(connect_socket, &auth, 1, 0);

	// if not authorized, exit and print error
	if (auth != '1'){
		fprintf(stderr, "%s: server refused connection\n", __FILE__);
		close(connect_socket);
		free(plaintext);
		free(key);
		exit(2);	
	}
	// otherwise send the size of the data
	else{
		char filesize[10];
		itoa(sizeof(plaintext), filesize, 10);
		send(connect_socket, filesize, sizeof(filesize), 0);
	}
	
	int size = strlen(plaintext);
	sendall(connect_socket, plaintext, &size);
	
	free(plaintext);
	free(key);
	
	close(connect_socket);
	return 0;
}

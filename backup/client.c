/*
 * client.c
 */

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

#define PORT "515151"
#define MAXDATASIZE 1000

void* get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){

	struct addrinfo hints, *servinfo, *traverse;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // change to AF_UNSPEC for IPv6 comp
	hints.ai_socktype = SOCK_STREAM;
	int rv;
	
	if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "%s: getaddrinfo error: %s\n", __FILE__, gai_strerror(rv));
		return 1;
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
	}

	freeaddrinfo(servinfo);
	char buf[MAXDATASIZE];
	int numbytes;
	send(connect_socket, "e#PLAINTEXT%KEYTEXTKEYTEXT@", 28, 0);
	
	if ((numbytes = recv(connect_socket, buf, MAXDATASIZE-1,0)) == -1){
		fprintf(stderr, "%s: recv error\n", __FILE__);
		exit(1);
	}
	
	buf[numbytes] = '\0';
	printf("%s received: '%s'\n", __FILE__, buf);
	close(connect_socket);
	return 0;
}

/*
 * server.c for TCP demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "515151"
#define MAXDATASIZE 4096
#define BACKLOG 5

void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

void* get_in_address(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return & (((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
	
	struct addrinfo hints, *servinfo, *traverse;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	int listen_sock;
	int yes=1;
	for (traverse=servinfo; traverse != NULL; traverse=traverse->ai_next){
		if ((listen_sock = socket(traverse->ai_family, traverse->ai_socktype,
				traverse->ai_protocol)) == -1){
			fprintf(stderr, "%s: socket\n", __FILE__);
			continue;
		}
		
		if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,&yes,
				sizeof(int)) == -1){
			fprintf(stderr, "%s: setsockopt\n", __FILE__);
			exit(1);
		}
		
		if (bind(listen_sock, traverse->ai_addr, traverse->ai_addrlen) == -1){
			close(listen_sock);
			fprintf(stderr, "%s: bind\n", __FILE__);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);
	
	if (traverse == NULL){
		fprintf(stderr, "%s: failed to bind\n");
		exit(1);
	}

	if (listen(listen_sock, BACKLOG) == -1){
		fprintf(stderr, "%s: listen error\n");
		exit(1);
	}

	// Reap zombies
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1){
		fprintf(stderr, "%s: sigaction error\n");
		exit(1);
	}
	
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int task_sock;
	char buffer[MAXDATASIZE];
	
	while(1){
		sin_size = sizeof(their_addr);
		task_sock = accept(listen_sock, (struct sockaddr*) &their_addr, &sin_size);
		if (task_sock == -1){
			fprintf(stderr, "%s: action socket error\n");
			continue;
		}
		
		inet_ntop(their_addr.ss_family, &their_addr, s, sizeof(s));
		if (!fork()){ // Child
			close(listen_sock);
			int chars_read;
			char id[2];
			
			// If not authorized, close connection
			recv(task_sock, id, 2, 0);
			id[1] = '\0';
			if( id[0] != 'e'){
				close(task_sock);
				exit(2);
			}
			else{
				send(task_sock, "1", 1, 0);
			}
			
			char size[5];
			memset(size, '\0', sizeof(size));
			recv(task_sock, size, sizeof(size) - 1, 0);

			char* buf = malloc(sizeof(char) * atoi(size));
			memset(buf, '\0', sizeof(char) * atoi(size));
			while((chars_read = recv(task_sock, buffer, MAXDATASIZE - 1, 0)) > 0){
				strcat(buf, buffer);
				memset(buffer, '\0', strlen(buffer));
			}
			
			strcat(buf, "\n");
			printf("%s", buf);
			close(task_sock);
			exit(0);
		}
		// Parent
		close(task_sock);
	} // Main Loop
	return 0;
}

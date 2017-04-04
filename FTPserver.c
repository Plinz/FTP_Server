/*
 * echoserverp.c - A pool version of echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256

#define HANDLER_PROCESS 40

int pid[HANDLER_PROCESS];

void echo(int connfd);

void handlerFin(int sig){
	for(int i = 0; i < HANDLER_PROCESS;i++){
		kill(pid[i],SIGINT);
	}	
	exit(0);
}



/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */

void handle(int listenfd){
	
	int connfd;
	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	clientlen = (socklen_t)sizeof(clientaddr);
	char client_hostname[MAX_NAME_LEN];
	char client_ip_string[INET_ADDRSTRLEN];
	
	while(1){
		while((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == -1){}
		
		// determine the name of the client 
		Getnameinfo((SA *) &clientaddr, clientlen,
		            client_hostname, MAX_NAME_LEN, 0, 0, 0);
		
		// determine the textual representation of the client's IP address 
		Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
		          INET_ADDRSTRLEN);
		

		getsockname(connfd, (struct sockaddr *)&clientaddr, &clientlen); 
		printf("server connected to %s (%s) port : %d addresse : %s, pid : %d\n", client_hostname, client_ip_string, ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr), getpid());
		
		
		echo(connfd);
		Close(connfd);
	}
}


int main(int argc, char **argv)
{

    int listenfd;
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(2121);
	signal(SIGINT,handlerFin);
	for(int i = 0 ; i < HANDLER_PROCESS;i++){
		if((pid[i] = fork()) == 0){
			signal(SIGINT,SIG_DFL);
			handle(listenfd);
			exit(0);
		}
	}

   	while (1) {
	}    
    exit(0);
}


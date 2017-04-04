#include "csapp.h"

#define MAX_NAME_LEN 256

#define HANDLER_PROCESS 40

int pid[HANDLER_PROCESS];
void connectClient(int clientfd);
void handlerFin(int sig){
	for(int i = 0; i < HANDLER_PROCESS;i++){
		kill(pid[i],SIGINT);
	}
	exit(0);
}

void handle(int listenfd){

	int masterfd,clientfd;
	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	clientlen = (socklen_t)sizeof(clientaddr);
	char client_hostname[MAX_NAME_LEN], client_ip_string[INET_ADDRSTRLEN], *bufContent;
	size_t bufContentSize;
	rio_t rio;

	while(1){
		while((masterfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == -1){}
		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
		Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
		getsockname(masterfd, (struct sockaddr *)&clientaddr, &clientlen);
		printf("Slave connected to %s (%s) port : %d addresse : %s, pid : %d\n", client_hostname, client_ip_string, ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr), getpid());

	    bufContent = (char*) malloc(MAXLINE);
	    Rio_readinitb(&rio, masterfd);
	    if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
	        printf("Slave received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);
			connectClient(clientfd=Open_clientfd(bufContent, 2123));
			Close(clientfd);
	        free(bufContent);
	    }
	}
}


int main(int argc, char **argv)
{

    int listenfd;
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(2122);
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

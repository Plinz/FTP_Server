#include "csapp.h"

#define MAX_NAME_LEN 256

#define HANDLER_PROCESS 5

int pid[HANDLER_PROCESS];
void connectClient(int clientfd);

void handlerFin(int sig){
	int i;
	printf("[SHUTDOWN] KILLING PROCESS\n");
	for(i = 0; i < HANDLER_PROCESS;i++){
		kill(pid[i],SIGINT);
		printf("[SHUTDOWN] PROCESS PID=%d KILLED\n", pid[i]);
	}
	printf("[SHUTDOWN] SHUTDOWN\n");
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

	    bufContent = (char*) malloc(MAXLINE);
	    Rio_readinitb(&rio, masterfd);
	    if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
			printf("[RUNNING][%d] NEW CONNEXION : HOSTNAME : %s\n", getpid(), bufContent);
			connectClient(clientfd=Open_clientfd(bufContent, 2123));
			Close(clientfd);
	        free(bufContent);
	    }
	}
}


int main(int argc, char **argv)
{

    int listenfd, i;
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }

	printf("[STARTING UP] INITIALIZATION OF THE SLAVE\n");
    listenfd = Open_listenfd(2122);
	signal(SIGINT,handlerFin);

	printf("[STARTING UP] LAUNCHING OF %d PROCESS\n", HANDLER_PROCESS);
	for(i = 0 ; i < HANDLER_PROCESS; i++){
		printf("[STARTING UP] LAUNCH PROCESS NUMBER %d\n", i);
		if((pid[i] = fork()) == 0){
			signal(SIGINT,SIG_DFL);
			handle(listenfd);
			exit(0);
		}
	}
	printf("[STARTING UP] END OF LAUNCH\n[RUNNING] READY TO LISTEN ON PORT 2121\n");
   	while (1) {}
    exit(0);
}

#include "csapp.h"

#define MAX_NAME_LEN 256
#define SLAVES_PROPERTIES "slaves.properties"
#define NB_SLAVES 3
#define HANDLER_PROCESS 40

int pid[HANDLER_PROCESS];

void echo(int connfd);

void handlerFin(int sig){
	for(int i = 0; i < HANDLER_PROCESS;i++){
		kill(pid[i],SIGINT);
	}
	exit(0);
}

char ** loadSlavesProperties(){
	char ** slaves = NULL;

	int index = 0;
	size_t bufContentSize = 0;
	ssize_t length;
	char *bufContent;
	FILE *prop = fopen(SLAVES_PROPERTIES, "r");
	if (prop == NULL)
		exit(EXIT_FAILURE);

	if((length = getline(&bufContent, &bufContentSize, prop)) != -1){
		slaves = malloc(sizeof(*slaves));
		if (slaves == NULL)
			exit(EXIT_FAILURE);
		slaves[index] = malloc(bufContentSize);
		strncpy(slaves[index], bufContent, bufContentSize);
		slaves[index][strlen(slaves[index])-1]='\0';
		index++;
		while((length = getline(&bufContent, &bufContentSize, prop)) != -1){
			slaves = realloc(slaves, sizeof(*slaves) * (index+1));
			if (slaves == NULL)
				exit(EXIT_FAILURE);
			slaves[index] = malloc(bufContentSize);
			strncpy(slaves[index], bufContent, bufContentSize);
			slaves[index][strlen(slaves[index])-1]='\0';
			index++;
		}
	}

	return slaves;
}

void handle(int listenfd, char** slaves){

	int slavefd;
	struct sockaddr_in clientaddr;
	socklen_t clientlen = (socklen_t)sizeof(clientaddr);
	char client_hostname[MAX_NAME_LEN];
	rio_t rioClient;
	int nextSlaves = 0;

	while(1){
		while(Accept(listenfd, (SA *)&clientaddr, &clientlen) == -1){}
		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
		slavefd = Open_clientfd(slaves[nextSlaves++], 2121);
		Rio_writen(slavefd, client_hostname, strlen(client_hostname));
		Close(slavefd);

		if(nextSlaves == NB_SLAVES)
			nextSlaves = 0;
	}
}


int main(int argc, char **argv)
{
	char **slaves;
    int listenfd;
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(2121);
	signal(SIGINT,handlerFin);
	slaves = loadSlavesProperties();

	for(int i = 0 ; i < HANDLER_PROCESS;i++){
		if((pid[i] = fork()) == 0){
			signal(SIGINT,SIG_DFL);
			handle(listenfd, slaves);
			exit(0);
		}
	}

   	while (1) {}
    exit(0);
}

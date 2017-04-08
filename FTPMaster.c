#include "csapp.h"

#define MAX_NAME_LEN 256
#define SLAVES_PROPERTIES "slaves.properties"
#define NB_SLAVES 3
#define HANDLER_PROCESS 5

int pid[HANDLER_PROCESS];

void echo(int connfd);

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

char ** loadSlavesProperties(){
	printf("[STARTING UP] LOAD SLAVES PROPERTIES\n");
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
		printf("[STARTING UP] LOAD SLAVES PROPERTIES : NEW SLAVE NUMBER %d HOSTNAME %s\n", index, slaves[index]);
		index++;
		while((length = getline(&bufContent, &bufContentSize, prop)) != -1){
			slaves = realloc(slaves, sizeof(*slaves) * (index+1));
			if (slaves == NULL)
				exit(EXIT_FAILURE);
			slaves[index] = malloc(bufContentSize);
			strncpy(slaves[index], bufContent, bufContentSize);
			slaves[index][strlen(slaves[index])-1]='\0';
			printf("[STARTING UP] LOAD SLAVES PROPERTIES : NEW SLAVE NUMBER %d HOSTNAME %s\n", index, slaves[index]);
			index++;
		}
	}

	return slaves;
}

void synchronize_Single_Slave(char* slave_Hostname, char* bufContent, size_t bufContentSize){
	int listenfd, slavefd;
	struct sockaddr_in slaveaddr;
	socklen_t slavelen = (socklen_t)sizeof(slaveaddr);

	char hostname[1024], *cmd;
	gethostname(hostname, 1024);
	slavefd = Open_clientfd(slave_Hostname, 2122);
	Rio_writen(slavefd, hostname, strlen(hostname));
	listenfd = Open_listenfd(2123);
	while((slavefd = Accept(listenfd, (SA *)&slaveaddr, &slavelen)) == -1){}
	cmd = strtok(bufContent, "  ");
	while (cmd != NULL){
		Rio_writen(slavefd, cmd, strlen(cmd));
	}
}

void synchronize_Slaves (int slavefd, char* slave_Source, char** slaves){
	char *bufContent;
	int i;
	size_t bufContentSize;
	rio_t rio;

	bufContent = (char*) malloc(MAXLINE);
	Rio_readinitb(&rio, slavefd);
	if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
		Close(slavefd);
		for(i=0; i<NB_SLAVES; i++)
			if(strcmp(slaves[i], slave_Source) != 0)
				synchronize_Single_Slave(slaves[i], bufContent, bufContentSize);
		free(bufContent);
	}
}

void handle(int listenfd, char** slaves){

	int slavefd, clientfd, nextSlaves, is_Slave;
	struct sockaddr_in clientaddr;
	socklen_t clientlen = (socklen_t)sizeof(clientaddr);
	char client_hostname[MAX_NAME_LEN];
	nextSlaves = 0;
	is_Slave = 0;

	while(1){
		while((clientfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) == -1){}
		Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

		printf("[RUNNING] NEW CONNEXION : HOSTNAME : %s HANDLE BY %s\n", client_hostname, slaves[nextSlaves]);

		// for(int i=0; i<NB_SLAVES; i++){
		// 	if (strcmp(slaves[i], client_hostname) == 0){
		// 		is_Slave = 1;
		// 		break;
		// 	}
		// }
		if (is_Slave){
			synchronize_Slaves(clientfd, client_hostname, slaves);
			Close(clientfd);
		}
		else {
			Close(clientfd);
			slavefd = Open_clientfd(slaves[nextSlaves++], 2122);
			Rio_writen(slavefd, client_hostname, strlen(client_hostname));
			Close(slavefd);
			if(nextSlaves == NB_SLAVES)
				nextSlaves = 0;
		}
	}
}


int main(int argc, char **argv)
{
	char **slaves;
    int listenfd, i;
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }


	printf("[STARTING UP] INITIALIZATION OF THE LOAD BALANCER\n");
    listenfd = Open_listenfd(2121);
	signal(SIGINT,handlerFin);
	slaves = loadSlavesProperties();

	printf("[STARTING UP] LAUNCHING OF %d PROCESS\n", HANDLER_PROCESS);
	for(i = 0 ; i < HANDLER_PROCESS;i++){
		printf("[STARTING UP] LAUNCH PROCESS NUMBER %d\n", i);
		if((pid[i] = fork()) == 0){
			signal(SIGINT,SIG_DFL);
			handle(listenfd, slaves);
			exit(0);
		}
	}
	printf("[STARTING UP] END OF LAUNCH\n[RUNNING] READY TO LISTEN ON PORT 2121\n");
   	while (1) {}
    exit(0);
}

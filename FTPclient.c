#include "csapp.h"
#include <string.h>
#include <time.h>
#include <libgen.h>

char pwd_FTP[MAXLINE];

void init_prompt(int clientfd,rio_t rio){
	char buf[MAXLINE];
	Rio_writen(clientfd, "PWD\n", 4);
	if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
		memcpy(pwd_FTP, buf, strlen(buf)-1);
	}
}

void display_prompt() {
	char * user = getenv("USER");
	printf("<%c[%d;%dmFTPServer %s%c[%dm> %s $ ", 27, 1, 33, user, 27, 0, pwd_FTP);
	fflush(stdout);
}

void toLower(char* str){
	for ( ; *str; ++str) *str = tolower(*str);
}

/* Affiche les informations à propos du transfert de fichier */
void printInfo(time_t start,time_t stop,int transfered){
	double duration = difftime(stop, start);
	double vitesse = (transfered/1000)/duration;
	if (duration == 0)
		printf("Fin du transfert de fichier : %d octets transférés. Vitesse trop rapide pour être calculée\n",transfered);
	else
		printf("Fin du transfert de fichier : %d octets transférés en %f secondes, vitesse :%f Kb/s\n",transfered, duration, vitesse);
}

/* Gestion du transfert de fichier */
void handleGetFile(char * filename,int clientfd,rio_t rio){

	char buf[MAXLINE],size[MAXLINE];
	time_t start, stop;
	size_t n;
	int transfered = 0;
	int size_To_Read;

	/* Construction de la commande à envoyer*/
	strcpy(buf,  "GET ");
	strcat(buf, filename);
	strcat(buf, "\n");

	/* Envoi de la commande GET nom_fichier au serveur */
	Rio_writen(clientfd, buf, strlen(buf));

	if ((n = Rio_readlineb(&rio, buf, 3)) != 0) {
		if (strcmp(buf,"OK") == 0){
			/* Calcul de la taille du fichier a recuperer */
			if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
				memcpy(size,buf,strlen(buf));
				size_To_Read = atoi(size);

				/*Debut du protocole de recuperation */
				FILE *fp = fopen(basename(filename), "w+");
				time(&start);
				while(transfered < size_To_Read) {
					n = Rio_readlineb(&rio, buf, MAXLINE);
					Rio_writen(fileno(fp), buf, n);
					transfered += n;
				}
				fclose(fp);
				time(&stop);
				printInfo(start,stop, transfered);

			}
		} else {
			/* Une erreur est survenue : recuperation de l'erreur */
			if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
				//memcpy(size,buf,strlen(buf)-1); NE marche pas, pourquoi mystere
				memcpy(size,buf,strlen(buf));
				size_To_Read = atoi(size);
	  		}
			if((Rio_readlineb(&rio, buf, size_To_Read))!=0){
				printf("Erreur serveur :%s\n",buf);
			}
			/*
			while(Rio_readlineb(&rio, buf, MAXLINE) > 0)
				printf("%s\n",buf);
			*/
		}
   } else
	   printf("Error : Server closed the connexion\n");
}

void handleBye(int clientfd){
	Rio_writen(clientfd, "BYE\n", 3);
}

void handleLS(int clientfd, rio_t rio){
	char buf[MAXLINE], size[MAXLINE];
	int size_To_Read;
	Rio_writen(clientfd, "LS\n", 3);
	if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
		printf("BUF : %s\n",buf);
		memcpy(size,buf,strlen(buf)-1);
		size_To_Read = atoi(size);
		printf("size=%d\n",size_To_Read);
		if((Rio_readnb(&rio, buf, (size_To_Read-1))) !=0){
			printf("%s\n",buf);
		} else {
			printf("ok\n");
		}
	}
	// while((Rio_readlineb(&rio, buf, MAXLINE))>0)
	// 	printf("%s",buf);
}

void handlePWD(int clientfd, rio_t rio){
	char buf[MAXLINE];
	Rio_writen(clientfd, "PWD\n", 4);
	if((Rio_readlineb(&rio, buf, MAXLINE))!=0)
		printf("%s",buf);
}

void handleMKDIR(char *dir, int clientfd, rio_t rio){
	char buf[MAXLINE], cmd[MAXLINE];
   	strcpy(cmd,  "MKDIR ");
	strcat(cmd, dir);
	strcat(cmd, "\n");
	Rio_writen(clientfd, cmd, strlen(cmd));
	if((Rio_readlineb(&rio, buf, MAXLINE))!=0)
		printf("%s",buf);
}


int main(int argc, char **argv)
{
	int listenfd, slavefd;
	struct sockaddr_in slaveaddr;
	socklen_t slavelen = (socklen_t)sizeof(slaveaddr);
	char *master, input[MAXLINE], finput[MAXLINE], *keyword;
	rio_t rio;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <host>\n", argv[0]);
		exit(0);
	}
	master = argv[1];

	Open_clientfd(master, 2121);
	listenfd = Open_listenfd(2123);
	while((slavefd = Accept(listenfd, (SA *)&slaveaddr, &slavelen)) == -1){}

	printf("client connected to server OS port : %d, addr : %s\n", ntohs(slaveaddr.sin_port), inet_ntoa(slaveaddr.sin_addr));
	Rio_readinitb(&rio, slavefd);
	init_prompt(slavefd,rio);
	display_prompt();

	while (Fgets(input, MAXLINE, stdin) != NULL) {
		memcpy(finput,input,strlen(input)-1);
		keyword = strtok(finput, " ");
		toLower(keyword);

		if(strcmp(keyword,"get") == 0){
			keyword = strtok(NULL, " ");
			handleGetFile(keyword,slavefd,rio);
		}
		else if(strcmp(keyword,"ls") == 0)
			handleLS(slavefd,rio);
		else if(strcmp(keyword,"pwd") == 0)
			handlePWD(slavefd,rio);
		// else if(strcmp(keyword,"cd") == 0)
		// 	handleCD(slavefd,rio);
		else if(strcmp(keyword,"mkdir") == 0){
			keyword = strtok(NULL, " ");
			handleMKDIR(keyword, slavefd, rio);
		}
		else if(strcmp(keyword,"bye") == 0){
			handleBye(slavefd);
			exit(0);
		}
		else
			printf("Commande inconnue : %s\n",keyword);
		display_prompt();
		memset(input,0,strlen(input));
		memset(finput,0,strlen(finput));
		memset(keyword,0,strlen(keyword));
	}
	Close(slavefd);
	exit(0);
}

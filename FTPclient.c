#include "csapp.h"
#include <string.h>
#include <time.h>
#include <libgen.h>

#define BLOCK_SIZE 1000000

char pwd_FTP[MAXLINE];

void handleERROR(rio_t rio){
	char buffer[MAXLINE];
	int size_To_Read;
	if((Rio_readlineb(&rio, buffer, MAXLINE))!=0)
		size_To_Read = atoi(buffer)+1;
	if((Rio_readlineb(&rio, buffer, size_To_Read))!=0)
		printf("Erreur serveur : %s\n",buffer);
}

void init_prompt(int slavefd,rio_t rio){
	memset(pwd_FTP,0,strlen(pwd_FTP));
	char buf[MAXLINE];
	int size_To_Read;
	Rio_writen(slavefd, "PWD\n", 4);
	if(Rio_readlineb(&rio, buf, 3) != 0){
		if (strcmp(buf,"OK") == 0){
			if((Rio_readlineb(&rio, buf, MAXLINE))!=0)
				size_To_Read = atoi(buf)+1;
			if((Rio_readlineb(&rio, buf, size_To_Read))!=0)
				memcpy(pwd_FTP, buf, strlen(buf));
		} else
			handleERROR(rio);
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

/* Gestion de récupération de fichier */
void handleGET(char * filename,int slavefd,rio_t rio){

	char buf[MAXLINE],size[MAXLINE];
	time_t start, stop;
	size_t n;
	int transfered = 0;
	int size_To_Read;

	/* Construction de la commande à envoyer*/
	sprintf(buf, "GET %s\n", filename);

	/* Envoi de la commande GET nom_fichier au serveur */
	Rio_writen(slavefd, buf, strlen(buf));

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
				memcpy(size,buf,strlen(buf));
				size_To_Read = atoi(size);
	  		}
			if((Rio_readlineb(&rio, buf, size_To_Read))!=0){
				printf("Erreur serveur : %s\n",buf);
			}
		}
   } else
	   printf("Error : Server closed the connexion\n");
}

void handlePUT(char * filename,int slavefd,rio_t rio){
	char bufContent[MAXLINE], size[MAXLINE];
	int taille;
	FILE *fp;
	size_t bufContentSize = strlen(filename);

	/* Construction de la commande à envoyer*/
	sprintf(bufContent, "PUT %s\n", filename);
	/* Envoi de la commande GET nom_fichier au serveur */
	Rio_writen(slavefd, bufContent, strlen(bufContent));


	int error = 0;
    if ((fp = fopen(filename, "r")) != NULL){

    	fseek(fp, 0L, SEEK_END);
    	taille = ftell(fp);
	    rewind(fp);
    	sprintf(size, "%d\n", taille);
    	Rio_writen(slavefd, size, strlen(size));
        filename = (char*) malloc(BLOCK_SIZE);

        while ((bufContentSize = fread(filename, sizeof(char), BLOCK_SIZE, fp)) > 0) {
            if (!ferror(fp))
                Rio_writen(slavefd, filename, bufContentSize);
            else
                Rio_writen(slavefd, "AN ERROR OCCURED DURING THE FILE READING\n", strlen("AN ERROR OCCURED DURING THE FILE READING\n"));
        }
    	free(filename);
    } else
        error = 1;
    if (error)
		printf("An error occured : %s\n", strerror(errno));

}

void handleCD(char* dir, int slavefd, rio_t rio){
	char cmd[MAXLINE];
	sprintf(cmd, "CD %s\n", dir);

	Rio_writen(slavefd, cmd, strlen(cmd));
	if(Rio_readlineb(&rio, cmd, 3) != 0){
		if (strcmp(cmd,"OK") == 0)
			init_prompt(slavefd, rio);
		else
			handleERROR(rio);

	}
}

void handleRM(char* file, int slavefd, rio_t rio){
	char cmd[MAXLINE];
	sprintf(cmd, "RM %s\n", file);

	Rio_writen(slavefd, cmd, strlen(cmd));
	if(Rio_readlineb(&rio, cmd, 3) != 0)
		if (strcmp(cmd,"KO") == 0)
			handleERROR(rio);
}

void handleRMR(char* dir, int slavefd, rio_t rio){
	char cmd[MAXLINE];
	sprintf(cmd, "RMR %s\n", dir);

	Rio_writen(slavefd, cmd, strlen(cmd));
	if(Rio_readlineb(&rio, cmd, 3) != 0)
		if (strcmp(cmd,"KO") == 0)
			handleERROR(rio);
}

void handleBYE(int slavefd){
	Rio_writen(slavefd, "BYE\n", 3);
}

void handleLS(int slavefd, rio_t rio){
	char buf[MAXLINE];
	int size_To_Read;
	Rio_writen(slavefd, "LS\n", 3);
	if(Rio_readlineb(&rio, buf, 3) != 0){
		if (strcmp(buf,"OK") == 0){
			if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
				size_To_Read = atoi(buf)-1;
				if((Rio_readnb(&rio, buf, size_To_Read)) !=0){
					printf("%s\n",buf);
				}
			}
		} else
			handleERROR(rio);
	}
	memset(buf,0,strlen(buf));
}

void handlePWD(int slavefd, rio_t rio){
	char buf[MAXLINE];
	int size_To_Read;
	Rio_writen(slavefd, "PWD\n", 4);
	if(Rio_readlineb(&rio, buf, 3) != 0){
		if (strcmp(buf,"OK") == 0){
			if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
				size_To_Read = atoi(buf)+1;
				if((Rio_readlineb(&rio, buf, size_To_Read))!=0)
					printf("%s\n",buf);
			}
		} else
			handleERROR(rio);
	}

}

void handleMKDIR(char *dir, int slavefd, rio_t rio){
	char cmd[MAXLINE];
	sprintf(cmd, "MKDIR %s\n", dir);

	Rio_writen(slavefd, cmd, strlen(cmd));
	if(Rio_readlineb(&rio, cmd, 3) != 0)
		if (strcmp(cmd,"KO") == 0)
			handleERROR(rio);
}


int main(int argc, char **argv)
{
	int listenfd, slavefd;
	struct sockaddr_in slaveaddr;
	socklen_t slavelen = (socklen_t)sizeof(slaveaddr);
	char *master, input[MAXLINE], finput[MAXLINE], *keyword;
	rio_t rio;

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <HOST>\n", argv[0]);
		exit(0);
	}
	master = argv[1];

	Open_clientfd(master, 2121);
	listenfd = Open_listenfd(2123);
	while((slavefd = Accept(listenfd, (SA *)&slaveaddr, &slavelen)) == -1){}

	printf("Server connexion : address : %s port : %d\n", inet_ntoa(slaveaddr.sin_addr), ntohs(slaveaddr.sin_port));
	Rio_readinitb(&rio, slavefd);
	init_prompt(slavefd,rio);
	display_prompt();

	while (Fgets(input, MAXLINE, stdin) != NULL) {
		memcpy(finput,input,strlen(input)-1);
		keyword = strtok(finput, " ");
		toLower(keyword);

		if(strcmp(keyword,"get") == 0){
			keyword = strtok(NULL, " ");
			handleGET(keyword,slavefd,rio);
		} else if(strcmp(keyword,"put") == 0){
			keyword = strtok(NULL, " ");
			handlePUT(keyword,slavefd,rio);
		} else if(strcmp(keyword,"ls") == 0)
			handleLS(slavefd,rio);
		else if(strcmp(keyword,"pwd") == 0)
			handlePWD(slavefd,rio);
		else if(strcmp(keyword,"cd") == 0){
			keyword = strtok(NULL, " ");
			handleCD(keyword, slavefd, rio);
		} else if(strcmp(keyword,"mkdir") == 0){
			keyword = strtok(NULL, " ");
			handleMKDIR(keyword, slavefd, rio);
		} else if(strcmp(keyword,"rm") == 0){
			keyword = strtok(NULL, " ");
			if (strcmp(keyword, "-r") == 0){
				keyword = strtok(NULL, " ");
				handleRMR(keyword, slavefd, rio);
			} else
				handleRM(keyword, slavefd, rio);
		} else if(strcmp(keyword,"bye") == 0){
			handleBYE(slavefd);
			exit(0);
		} else
			printf("Unknow command : %s\n",keyword);
		display_prompt();
		memset(input,0,strlen(input));
		memset(finput,0,strlen(finput));
		memset(keyword,0,strlen(keyword));
	}
	Close(slavefd);
	exit(0);
}

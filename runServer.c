/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void ls(int clientfd){
	pid_t pid;
	Rio_writen(clientfd, "OK-", strlen("OK"));
	if((pid = fork())==0){
		if(dup2(clientfd,1)== -1){
			printf("Erreur dup");
		}
		char * retour[2]={"ls",NULL};
		if(execvp(retour[0],retour) == -1){
			printf("Error\n");
		}
	}	
	waitpid(-1,NULL,0);
	printf("Fin\n");
}

void pwd(int clientfd){
	pid_t pid;
	Rio_writen(clientfd, "OK-", strlen("OK"));
	if((pid = fork())==0){
		if(dup2(clientfd,1)== -1){
			printf("Erreur dup");
		}
		char * retour[2]={"pwd",NULL};
		if(execvp(retour[0],retour) == -1){
			printf("Error\n");
		}
	}	
	waitpid(-1,NULL,0);
	printf("Fin\n");
}

void my_mkdir(char * bufContent, int clientfd){
	pid_t pid;
	Rio_writen(clientfd, "OK-", strlen("OK"));
	if((pid = fork())==0){
		if(dup2(clientfd,1)== -1){
			printf("Erreur dup");
		}
		char * retour[3]={"mkdir",bufContent,NULL};
		if(execvp(retour[0],retour) == -1){
			printf("Error\n");
		}
	}	
	waitpid(-1,NULL,0);
	printf("Fin\n");
}

void fileTransfer(char * bufContent,int clientfd){
	size_t bufContentSize = strlen(bufContent);
	bufContent[bufContentSize-1]='\0';
    	FILE *fp = fopen(bufContent, "r");
	int error = 0;
	printf("buffContent : %s\n",bufContent);
        if (fp != NULL){
            Rio_writen(clientfd, "OK-", strlen("OK"));
            bufContent = (char*) malloc(BLOCK_SIZE);
            while ((bufContentSize = fread(bufContent, sizeof(char), BLOCK_SIZE, fp)) > 0) {
                if (!ferror(fp))
                    Rio_writen(clientfd, bufContent, bufContentSize);
                else
                    Rio_writen(clientfd, "AN ERROR OCCURED DURING THE FILE READING\n", strlen("AN ERROR OCCURED DURING THE FILE READING\n"));
            }
        } else
            error = 1;
        if (error){
            char *strerr = strerror(errno);
            Rio_writen(clientfd, "KO", strlen("KO"));
            Rio_writen(clientfd, strerr, strlen(strerr));
        }
        free(bufContent);

}


void connectClient(int clientfd)
{
    size_t bufContentSize;
    char *bufContent,*commande;
    rio_t rio;
    

    bufContent = (char*) malloc(MAXLINE);
    Rio_readinitb(&rio, clientfd);
    if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
        printf("server received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);

	
	commande  = strtok(bufContent, " ");
	if(strcmp(commande,"GET")==0){
		commande = strtok(NULL, " ");
		fileTransfer(commande,clientfd);
	}
	else if(strcmp(commande,"LS\n")==0){
		ls(clientfd);
	}
	else if(strcmp(commande,"PWD\n")==0){
		pwd(clientfd);
	}		
	else if(strcmp(commande,"MKDIR")==0){
		commande = strtok(NULL, " ");
		my_mkdir(commande,clientfd);
	}
    }
}

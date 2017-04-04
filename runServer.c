#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void toLowerString(char* str){
	for ( ; *str; ++str) *str = tolower(*str);
}

void my_LS(int clientfd){
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

void my_PWD(int clientfd){
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

void my_MKDIR(char * bufContent, int clientfd){
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

void getFile(char * bufContent,int clientfd){
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
    char bufContent[MAXLINE], finput[MAXLINE], *keyword;
    rio_t rio;

    Rio_readinitb(&rio, clientfd);
    if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
        printf("server received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);
		memcpy(finput,bufContent,strlen(bufContent)-1);
		keyword = strtok(bufContent, " ");
		toLowerString(keyword);

		if(strcmp(keyword,"get")==0)
			getFile(strtok(NULL, " "),clientfd);
		else if(strcmp(keyword,"ls")==0)
			my_LS(clientfd);
		else if(strcmp(keyword,"pwd")==0)
			my_PWD(clientfd);
		else if(strcmp(keyword,"mkdir")==0)
			my_MKDIR(strtok(NULL, " "),clientfd);
		// else if(strcmp(keyword,"bye") == 0)
		// 	break;
    }
}

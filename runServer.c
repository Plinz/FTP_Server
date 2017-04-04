/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000
void ls(int connfd){
	pid_t pid;

	//pipe(tube);
	if((pid = fork())==0){
		printf("Ecriture du LS gros\n");
		dup2(1,connfd);
		char * retour[2]={"ls",NULL};
		execvp(retour[0],retour);
	}	
	waitpid(pid,NULL,0);
}


void connectClient(int clientfd)
{
    size_t bufContentSize;
    char *bufContent;
    rio_t rio;
    int error = 0;

    Rio_writen(clientfd, "Esclave pour vous servir", strlen("Esclave pour vous servir"));

    bufContent = (char*) malloc(MAXLINE);
    Rio_readinitb(&rio, clientfd);
    if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXLINE)) != 0) {
        printf("server received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);
    	bufContent[bufContentSize-1]='\0';
    	FILE *fp = fopen(bufContent, "r");

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
}

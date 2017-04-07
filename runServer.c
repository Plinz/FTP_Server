#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void my_LS(int clientfd){
	char buf_ls[MAXLINE], size[MAXLINE];
	int out_pipe[2];
	if(pipe(out_pipe) != 0) {exit(1);}
	if(fork()==0){
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(out_pipe[1], STDERR_FILENO);
		Close(out_pipe[1]);
		char * ls[2]={"ls",NULL};
		if(execvp(ls[0],ls) == -1){
			printf("Error\n");
		}
	} else{
		Close(out_pipe[1]);
		waitpid(-1,NULL,0);
		read(out_pipe[0], buf_ls, MAXLINE);
		Close(out_pipe[0]);
		sprintf(size, "%lu\n", strlen(buf_ls));
		Rio_writen(clientfd, size, strlen(size));
		Rio_writen(clientfd, buf_ls, strlen(buf_ls));
	}
}

void my_PWD(int clientfd){
	char * pwd[2]={"pwd",NULL};
	char buffer[MAXLINE];
	int out_pipe[2];
	if(pipe(out_pipe) != 0) {exit(1);}
	if(fork()==0){
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(out_pipe[1], STDERR_FILENO);
		Close(out_pipe[1]);
		if(execvp(pwd[0],pwd) == -1){
			printf("Error\n");
		}
	} else{
		Close(out_pipe[1]);
		waitpid(-1,NULL,0);
		read(out_pipe[0], buffer, MAXLINE);
		Close(out_pipe[0]);
		Rio_writen(clientfd, buffer, strlen(buffer));
	}
}

void my_MKDIR(char* bufContent, int clientfd){
	char * mkdir[3]={"mkdir",bufContent,NULL};
	char buffer[MAXLINE];
	int out_pipe[2], resultSize;
	int saved_stdout = dup(STDOUT_FILENO);
	if(pipe(out_pipe) != 0) {exit(1);}
	if(fork()==0){
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(out_pipe[1], STDERR_FILENO);
		Close(out_pipe[1]);
		if(execvp(mkdir[0],mkdir) == -1){
			printf("Error\n");
		}
	} else{
		Close(out_pipe[1]);
		waitpid(-1,NULL,0);
		resultSize = read(out_pipe[0], buffer, MAXLINE);
		Close(out_pipe[0]);
		dup2(saved_stdout, STDOUT_FILENO);
		if (resultSize == 0)
			Rio_writen(clientfd, "Directory successfully created.\n", strlen("Directory successfully created.\n"));
		else
			Rio_writen(clientfd, buffer, resultSize);
	}
}

void getFile(char * bufContent,int clientfd){
	char size[MAXLINE];
	int taille;
	FILE *fp;
	size_t bufContentSize = strlen(bufContent);
	//bufContent[bufContentSize-1]='\0'; Inutile

	int error = 0;
        if ((fp = fopen(bufContent, "r")) != NULL){

	    /* Calcul de la taille du fichier a envoyer et envoi au client*/


	    /* Debut du protocole de transfert */
            Rio_writen(clientfd, "OK", strlen("OK"));
	    fseek(fp, 0L, SEEK_END);
	    taille = ftell(fp);
 	    rewind(fp);
	    sprintf(size, "%d\n", taille);
	    Rio_writen(clientfd, size, strlen(size));
            bufContent = (char*) malloc(BLOCK_SIZE);

            while ((bufContentSize = fread(bufContent, sizeof(char), BLOCK_SIZE, fp)) > 0) {
                if (!ferror(fp))
                    Rio_writen(clientfd, bufContent, bufContentSize);
                else
                    Rio_writen(clientfd, "AN ERROR OCCURED DURING THE FILE READING\n", strlen("AN ERROR OCCURED DURING THE FILE READING\n"));
            }
	    free(bufContent);
        } else
            error = 1;
        if (error){

	    /* Debut du protocole d'erreur */
            char *strerr = strerror(errno);
	    printf("An error occured : %s + %lu\n",strerr,strlen(strerr));
            Rio_writen(clientfd, "KO", strlen("KO"));

	    /* Envoi de l'erreur survenue*/
	    sprintf(size, "%lu\n", strlen(strerr));
	    printf("Size : %s\n",size);

	    Rio_writen(clientfd, size, strlen(size));
            Rio_writen(clientfd, strerr, strlen(strerr));
        }
}


void connectClient(int clientfd)
{
    size_t bufContentSize;
    char bufContent[MAXLINE], finput[MAXLINE], *keyword;
    rio_t rio;


    Rio_readinitb(&rio, clientfd);
	while (1){
		if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXBUF)) != 0) {
	        printf("Slave received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);
			strncpy(finput,bufContent,strlen(bufContent)-1);
			keyword = strtok(finput, " ");
			if(strcmp(keyword, "GET")==0){
				keyword = strtok(NULL, " ");
				printf("KEYWORD : %s \n",keyword);
				getFile(keyword, clientfd);
			}
			else if(strcmp(keyword, "LS")==0)
				my_LS(clientfd);
			else if(strcmp(keyword, "PWD")==0)
				my_PWD(clientfd);
			else if(strcmp(keyword, "MKDIR")==0){
				keyword = strtok(NULL, " ");
				my_MKDIR(keyword, clientfd);
			}
			 else if(strcmp(keyword,"BYE") == 0){
			 	break;
			}
		}
		memset(bufContent,0,strlen(bufContent));
		memset(finput,0,strlen(finput));
		memset(keyword,0,strlen(keyword));
	}
}

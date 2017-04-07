#include "csapp.h"
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void send_Error(char* error, int clientfd){
	char size[MAXLINE];
	Rio_writen(clientfd, "KO", strlen("KO"));
	sprintf(size, "%lu\n", strlen(error));
	Rio_writen(clientfd, size, strlen(size));
	Rio_writen(clientfd, error, strlen(error));
}

void my_LS2(int clientfd){
	char buf_ls[MAXLINE], size[MAXLINE];
	int out_pipe[2], size_Readed;
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
		size_Readed = read(out_pipe[0], buf_ls, MAXLINE);
		Close(out_pipe[0]);
		if (size_Readed != 0){
			Rio_writen(clientfd, "OK", strlen("OK"));
			sprintf(size, "%lu\n", strlen(buf_ls));
			Rio_writen(clientfd, size, strlen(size));
			Rio_writen(clientfd, buf_ls, strlen(buf_ls));
		} else
		Rio_writen(clientfd, "KO", strlen("KO"));
	}
}

void my_LS(int clientfd){
	char buf[MAXLINE], name[MAXLINE], size[MAXLINE];
	DIR *dp;
	struct dirent *ep;
	dp = opendir ("./");

	if (dp != NULL)	{
		while ((ep = readdir (dp)) != NULL){
			sprintf(name, "%s\n", ep->d_name);
			if(strlen(name) + strlen(buf) < MAXLINE)
				strcat(buf,name);
		}
		(void) closedir (dp);
	}
	Rio_writen(clientfd, "OK", strlen("OK"));
	sprintf(size, "%lu\n", strlen(buf));
	Rio_writen(clientfd, size, strlen(size));
	Rio_writen(clientfd, buf, strlen(buf));
}

void my_PWD(int clientfd){
	char cwd[MAXLINE], size[MAXLINE];
	if (getcwd(cwd, sizeof(cwd)) != NULL){
		Rio_writen(clientfd, "OK", strlen("OK"));
		sprintf(size, "%lu\n", strlen(cwd));
		Rio_writen(clientfd, size, strlen(size));
		Rio_writen(clientfd, cwd, strlen(cwd));
	} else
		send_Error(strerror(errno), clientfd);
}

void my_MKDIR(char* bufContent, int clientfd){
	int resultSize = mkdir(bufContent, 0775);
	if (resultSize == 0)
		Rio_writen(clientfd, "OK", strlen("OK"));
	else
		send_Error(strerror(errno), clientfd);
}

void my_RM(char* bufContent, int clientfd){
	int resultSize = unlink(bufContent);
	if (resultSize == 0)
		Rio_writen(clientfd, "OK", strlen("OK"));
	else
		send_Error(strerror(errno), clientfd);
}

void my_RMR(char* bufContent, int clientfd){
	int resultSize = rmdir(bufContent);
	if (resultSize == 0)
		Rio_writen(clientfd, "OK", strlen("OK"));
	else
		send_Error(strerror(errno), clientfd);
}

void my_CD(char* bufContent, int clientfd){
	int resultSize = chdir(bufContent);
	if (resultSize == 0)
		Rio_writen(clientfd, "OK", strlen("OK"));
	else
		send_Error(strerror(errno), clientfd);
}

void getFile(char * bufContent,int clientfd){
	char size[MAXLINE];
	int taille;
	FILE *fp;
	size_t bufContentSize = strlen(bufContent);

	int error = 0;
        if ((fp = fopen(bufContent, "r")) != NULL){
	    	/* Debut du protocole de transfert */
            Rio_writen(clientfd, "OK", strlen("OK"));
			/* Calcul de la taille du fichier a envoyer et envoi au client*/

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
		    printf("[RUNNING][%d] An error occured : %s\n",getpid(), strerr);
	        Rio_writen(clientfd, "KO", strlen("KO"));

		    /* Envoi de l'erreur survenue*/
		    sprintf(size, "%lu\n", strlen(strerr));
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
			printf("[RUNNING][%d] SIZE : %luB CONTENT : %s", getpid(), bufContentSize, bufContent);
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
			} else if(strcmp(keyword, "CD")==0){
				keyword = strtok(NULL, " ");
				my_CD(keyword, clientfd);
			} else if(strcmp(keyword, "RM")==0){
				keyword = strtok(NULL, " ");
				my_RM(keyword, clientfd);
			} else if(strcmp(keyword, "RMR")==0){
				keyword = strtok(NULL, " ");
				my_RMR(keyword, clientfd);
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

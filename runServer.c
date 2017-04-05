#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void my_LS(int clientfd){
	char buffer[MAXLINE], size[MAXLINE];
	int out_pipe[2];
	if(pipe(out_pipe) != 0) {exit(1);}
	if(fork()==0){
		dup2(out_pipe[1], STDOUT_FILENO);
		Close(out_pipe[1]);
		char * ls[2]={"ls",NULL};
		if(execvp(ls[0],ls) == -1){
			printf("Error\n");
		}
	} else{
		Close(out_pipe[1]);
		waitpid(-1,NULL,0);
		fflush(stdout);
		read(out_pipe[0], buffer, MAXLINE);
		Close(out_pipe[0]);
		sprintf(size, "%lu\n", strlen(buffer));
		Rio_writen(clientfd, size, strlen(size));
		Rio_writen(clientfd, buffer, strlen(buffer));
	}
}

void my_CMD(int clientfd, char** cmd){
	char buffer[MAXLINE];
	int out_pipe[2];
	if(pipe(out_pipe) != 0) {exit(1);}
	if(fork()==0){
		dup2(out_pipe[1], STDOUT_FILENO);
		Close(out_pipe[1]);
		if(execvp(cmd[0],cmd) == -1){
			printf("Error\n");
		}
	} else{
		Close(out_pipe[1]);
		waitpid(-1,NULL,0);
		fflush(stdout);
		read(out_pipe[0], buffer, MAXLINE);
		Close(out_pipe[0]);
		Rio_writen(clientfd, buffer, strlen(buffer));
	}
}

void my_MKDIR(char * bufContent, int clientfd){
	pid_t pid;
	Rio_writen(clientfd, "OK-\n", strlen("OK-\n"));
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
}

void getFile(char * bufContent,int clientfd){
	size_t bufContentSize = strlen(bufContent);
	bufContent[bufContentSize-1]='\0';
    FILE *fp = fopen(bufContent, "r");
	int error = 0;
	printf("buffContent : %s\n",bufContent);
        if (fp != NULL){
            Rio_writen(clientfd, "OK-", strlen("OK-"));
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
	char * pwd[2]={"pwd",NULL};



    Rio_readinitb(&rio, clientfd);
	while (1){
		if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXBUF)) != 0) {
	        printf("Slave received %u bytes && contenu : %s\n", (unsigned int)bufContentSize, bufContent);
			strncpy(finput,bufContent,strlen(bufContent)-1);
			keyword = strtok(finput, " ");
			if(strcmp(keyword,"GET")==0)
				getFile(strtok(NULL, " "),clientfd);
			else if(strcmp(keyword,"LS")==0){
				my_LS(clientfd);
			}
			else if(strcmp(keyword,"PWD")==0)
				my_CMD(clientfd, pwd);
			else if(strcmp(keyword,"MKDIR")==0)
				my_MKDIR(strtok(NULL, " "),clientfd);
			// else if(strcmp(keyword,"bye") == 0)
			// 	break;
		}
		memset(bufContent,0,strlen(bufContent));
		memset(finput,0,strlen(finput));
		memset(keyword,0,strlen(keyword));
	}
}

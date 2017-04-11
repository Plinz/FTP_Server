#include "csapp.h"
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#define BLOCK_SIZE 1000000

int isConnect;

void send_String(char* buf, int clientfd){
	char size[MAXLINE];
	sprintf(size, "%lu\n", strlen(buf));
	Rio_writen(clientfd, size, strlen(size));
	Rio_writen(clientfd, buf, strlen(buf));
}

void send_Buffer(char* buf, int clientfd){
	Rio_writen(clientfd, "OK", strlen("OK"));
	send_String(buf, clientfd);
}

void send_Error(char* error, int clientfd){
	printf("[RUNNING][%d] An error occured : %s\n",getpid(), error);
	Rio_writen(clientfd, "KO", strlen("KO"));
	send_String(error, clientfd);
}

void my_LS(int clientfd){
	char buf[MAXLINE], name[MAXLINE];
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
	send_Buffer(buf, clientfd);
	memset(buf,0,strlen(buf));
}

void my_PWD(int clientfd){
	char cwd[MAXLINE];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		send_Buffer(cwd, clientfd);
	else
		send_Error(strerror(errno), clientfd);
}

void my_AUTH(char *login, char *password, int clientfd){
	char line[MAXLINE], *l, *p;
	FILE *fp;
	int find = 0;
	if ((fp = fopen("users", "r")) != NULL){
		while(!find && (fgets(line, sizeof(line), fp) != NULL)){
			l = strtok(line, " ");
			p = strtok(NULL, " ");
			p[strlen(p)-1] = '\0';
			if (!strcmp(l, login) && !strcmp(p, password))
				find = 1;
		}
	}
	if (find){
		isConnect = 1;
		Rio_writen(clientfd, "OK", strlen("OK"));
	}
	else
		Rio_writen(clientfd, "KO", strlen("KO"));

}

int my_Simple_Command(int size, int clientfd){
	if (size == 0){
		Rio_writen(clientfd, "OK", strlen("OK"));
		return strlen("OK");
	} else{
		send_Error(strerror(errno), clientfd);
		return -1;
	}
}

void my_GET(char * bufContent,int clientfd){
	char size[MAXLINE];
	int taille;
	FILE *fp;
	size_t bufContentSize = strlen(bufContent);

    if ((fp = fopen(bufContent, "r")) != NULL){
    	/* Debut du protocole de transfert */
        Rio_writen(clientfd, "OK", strlen("OK"));
		/* Calcul de la taille du fichier a envoyer et envoi au client*/

    	if (fseek(fp, 0L, SEEK_END) == 0){
	    	if ((taille = ftell(fp)) != -1){
			rewind(fp);
		    	sprintf(size, "%d\n", taille);
		    	Rio_writen(clientfd, size, strlen(size));
		        bufContent = (char*) malloc(BLOCK_SIZE);
		        while ((bufContentSize = fread(bufContent, sizeof(char), BLOCK_SIZE, fp)) > 0) {
		            if (!ferror(fp))
		                Rio_writen(clientfd, bufContent, bufContentSize);
		            else {
		                Rio_writen(clientfd, "AN ERROR OCCURED DURING THE FILE READING\n", strlen("AN ERROR OCCURED DURING THE FILE READING\n"));
						break;
					}
		        }
			} else
				send_Error(strerror(errno), clientfd);
		} else
			send_Error(strerror(errno), clientfd);
    } else
        send_Error(strerror(errno), clientfd);
	free(bufContent);
}

int my_PUT(char * filename, int clientfd, rio_t rio){

	char buf[MAXLINE],size[MAXLINE];
	size_t n;
	int size_To_Read, transfered = 0;
	FILE *fp;
	/* Calcul de la taille du fichier a recuperer */
	if((Rio_readlineb(&rio, buf, MAXLINE))!=0){
		memcpy(size,buf,strlen(buf));
		size_To_Read = atoi(size);

		if ((fp = fopen(basename(filename), "w+")) != NULL){
			while(transfered < size_To_Read) {
				n = Rio_readlineb(&rio, buf, MAXLINE);
				Rio_writen(fileno(fp), buf, n);
				transfered += n;
			}
			fclose(fp);
			return transfered;
		} else
			send_Error(strerror(errno), clientfd);
	}
	return -1;
}

void synchronize(char *command, char *keyword, char *masterHost){
	char syncro[MAXLINE], cwd[MAXLINE];

	sprintf(syncro, "%s %s/%s\n", command, getcwd(cwd, sizeof(cwd)), keyword);
	int masterfd = Open_clientfd(masterHost, 2121);
	Rio_writen(masterfd, syncro, strlen(syncro));
	Close(masterfd);
}


void connectClient(int clientfd, char* masterHost)
{
    size_t bufContentSize;
    char bufContent[MAXLINE], finput[MAXLINE], *keyword, *password;
    rio_t rio;

	isConnect = 0;

    Rio_readinitb(&rio, clientfd);
	while (1){
		if ((bufContentSize = Rio_readlineb(&rio, bufContent, MAXBUF)) != 0) {
			printf("[RUNNING][%d] SIZE : %luB CONTENT : %s", getpid(), bufContentSize, bufContent);
			strncpy(finput,bufContent,strlen(bufContent)-1);
			keyword = strtok(finput, " ");
			if(strcmp(keyword, "GET")==0){
				keyword = strtok(NULL, " ");
				my_GET(keyword, clientfd);
			} else if(strcmp(keyword, "PUT")==0){
				if (isConnect){
					keyword = strtok(NULL, " ");
					if (my_PUT(keyword, clientfd, rio) != -1)
						synchronize("PUT", keyword, masterHost);
				} else
					send_Error("Permission denied\n", clientfd);
			} else if(strcmp(keyword, "LS")==0)
				my_LS(clientfd);
			else if(strcmp(keyword, "PWD")==0)
				my_PWD(clientfd);
			else if(strcmp(keyword, "MKDIR")==0){
				if (isConnect){
					keyword = strtok(NULL, " ");
					if (my_Simple_Command(mkdir(keyword, 0775), clientfd) != -1)
						synchronize("MKDIR", keyword, masterHost);
				} else
					send_Error("Permission denied\n", clientfd);
			} else if(strcmp(keyword, "CD")==0){
				keyword = strtok(NULL, " ");
				my_Simple_Command(chdir(keyword), clientfd);
			} else if(strcmp(keyword, "RM")==0){
				if (isConnect){
					keyword = strtok(NULL, " ");
					if (my_Simple_Command(unlink(keyword), clientfd) != -1)
						synchronize("RM", keyword, masterHost);
				} else
					send_Error("Permission denied\n", clientfd);
			} else if(strcmp(keyword, "RMR")==0){
				if (isConnect){
					keyword = strtok(NULL, " ");
					if (my_Simple_Command(rmdir(keyword), clientfd) != -1)
						synchronize("RMR", keyword, masterHost);
				} else
					send_Error("Permission denied\n", clientfd);
			} else if(strcmp(keyword, "AUTH")==0){
				keyword = strtok(NULL, " ");
				password = strtok(NULL, " ");
				my_AUTH(keyword, password, clientfd);
				memset(password,0,strlen(password));
			} else if(strcmp(keyword,"BYE") == 0){
			 	break;
			}
		}
		memset(bufContent,0,strlen(bufContent));
		memset(finput,0,strlen(finput));
		memset(keyword,0,strlen(keyword));
	}
}

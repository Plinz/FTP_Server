/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <string.h>
#include <time.h>
#include <libgen.h>

/* Affiche les informations à propos du transfert de fichier */

void handleLS(){



}


void printInfo(time_t start,time_t stop,int transfered){

	double duration = difftime(stop, start);
        double vitesse = (transfered/1000)/duration;
        if (duration == 0)
        	printf("Fin du transfert de fichier : %d octets transférés. Vitesse trop rapide pour être calculée\n",transfered);
        else
        	printf("Fin du transfert de fichier : %d octets transférés en %f secondes, vitesse :%f Kb/s\n",transfered, duration, vitesse);

}

/* Gestion du transfert de fichier */
void handleFileTransfer(char * buf,int clientfd,rio_t rio){

	char * token,filename[MAXLINE];
	int transfered;
    	time_t start, stop;
	size_t n;

	memcpy(filename, buf, strlen(buf)-1);
        Rio_writen(clientfd, buf, strlen(buf));
	transfered = 0;

        if ((n = Rio_readlineb(&rio, buf, 3)) != 0) {

            if (strcmp(buf,"OK") == 0){
                printf("toto:%s\n",buf);
            } else {
                printf("tata:%s\n",buf);
            }

            token = strtok(buf, "-");
            if (strcmp(token, "OK") == 0){
                while( token != NULL ) {
                    printf( "%s\n", token );
                    token = strtok(NULL, "-");
                }

            	FILE *fp = fopen(basename(filename), "w+");

            	time(&start);

                while((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
            		Rio_writen(fileno(fp), buf, n);
                    transfered += n;
                }
            	fclose(fp);
                time(&stop);

                printInfo(start,stop, transfered);

            } else {
                while( token != NULL ) {
                    printf("Erreur Serveur :\n");
                    printf( "%s\n", token );
                    token = strtok(NULL, "-");
                }
            }
        }
}

int main(int argc, char **argv)
{
    int clientfd;
    
    char *host, buf[MAXLINE], *text;
    rio_t rio;


    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    clientfd = Open_clientfd(host, 2121);
   

    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    getsockname(clientfd, (struct sockaddr *)&clientaddr, &clientlen);
    printf("client connected to server OS port : %d, addr : %s\n", ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr));

    Rio_readinitb(&rio, clientfd);

    if (Fgets(buf, MAXLINE, stdin) != NULL) {
	
	text = strtok(buf, " ");
	printf("Test ;;;;;;;;  %s et %s\n",text);
	if(strcmp(text,"GET")==0){
		text = strtok(NULL, " ");;
		printf("BUF size : %ld : text : %s \n",strlen(text),text);
		handleFileTransfer(text,clientfd,rio);
	}
	else if(strcmp(text,"LS")==0)
		printf("BUF size : %ld : text : %s \n",strlen(text),text);
		
    }
    Close(clientfd);
    exit(0);
}

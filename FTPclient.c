/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <string.h>
#include <time.h>
#include <libgen.h>

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
            	//fclose(fp); Ca bug sa race avec cette merde.
                time(&stop);
                printInfo(start,stop, transfered);

            } else {
		printf("Erreur\n");
                while( token != NULL ) {
                    printf("Erreur Serveur :\n");
                    printf( "%s\n", token );
                    token = strtok(NULL, "-");
                }
            }
        }
}

void handleLS(int clientfd,rio_t rio){
	char  buf2[MAXLINE];
        Rio_writen(clientfd, "LS\n", 3);
	while((Rio_readlineb(&rio, buf2, MAXLINE))>0)
		printf("%s",buf2);
}

void handlePWD(int clientfd,rio_t rio){
	char  buf2[MAXLINE];
        Rio_writen(clientfd, "PWD\n", 4);
	while((Rio_readlineb(&rio, buf2, MAXLINE))>0)
		printf("%s",buf2);
}

void handleMKDIR(char * buf,int clientfd,rio_t rio){
	Rio_writen(clientfd,buf,strlen(buf));

}


int main(int argc, char **argv)
{
    int listenfd, slavefd;
	struct sockaddr_in slaveaddr;
	socklen_t slavelen = (socklen_t)sizeof(slaveaddr);
    char *master, buf[MAXLINE],*commande, *copy;
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

    if (Fgets(buf, MAXLINE, stdin) != NULL) {
	commande = (char *)malloc(strlen(buf));
	memcpy(commande,buf,strlen(buf));
	copy = buf;
	copy = strtok(copy, " ");
	
    	if(strcmp(copy,"GET")==0){
		handleFileTransfer(commande,slavefd,rio);
	}
	else if(strcmp(copy,"LS\n")==0)
		handleLS(slavefd,rio);

	else if(strcmp(copy,"PWD\n")==0){
		handlePWD(slavefd,rio);
	}
	else if(strcmp(copy,"MKDIR")==0)
		handleMKDIR(commande, slavefd,rio);
	printf("Fin du transfert");
    }
    Close(slavefd);
    exit(0);
}

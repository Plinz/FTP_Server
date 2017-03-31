/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <string.h>
#include <time.h>
#include <libgen.h>

int main(int argc, char **argv)
{
    int clientfd, transfered;
    size_t n;
    char *host, *token, buf[MAXLINE], filename[MAXLINE];
    rio_t rio;
    double duration, vitesse;
    time_t start, stop;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    clientfd = Open_clientfd(host, 2121);
    transfered = 0;

    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    getsockname(clientfd, (struct sockaddr *)&clientaddr, &clientlen);
    printf("client connected to server OS port : %d, addr : %s\n", ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr));

    Rio_readinitb(&rio, clientfd);

    if (Fgets(buf, MAXLINE, stdin) != NULL) {

        memcpy(filename, buf, strlen(buf)-1);
        Rio_writen(clientfd, buf, strlen(buf));

        if ((n = Rio_readlineb(&rio, buf, 3)) != 0) {

            if (!strcmp(buf,"OK")){
                printf("toto:%s",buf);
            } else {
                printf("tata:%s",buf);
            }

            token = strtok(buf, "-");
            if (!strcmp(token, "OK")){
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
                duration = difftime(stop, start);
                vitesse = (transfered/1000)/duration;
                if (duration == 0)
                    printf("Fin du transfert de fichier : %d octets transférés. Vitesse trop rapide pour être calculé\n",transfered);
                else
                    printf("Fin du transfert de fichier : %d octets transférés en %f secondes, vitesse :%f Kb/s\n",transfered, duration, vitesse);

            } else {
                while( token != NULL ) {
                    printf("Erreur Serveur :\n");
                    printf( "%s\n", token );
                    token = strtok(NULL, "-");
                }
            }
        }
    }
    Close(clientfd);
    exit(0);
}

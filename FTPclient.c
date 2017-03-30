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
    char *host, buf[MAXLINE], filename[MAXLINE];
    rio_t rio;
    int duration;
    time_t start, stop;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, 2121);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */

    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    getsockname(clientfd, (struct sockaddr *)&clientaddr, &clientlen); 
    printf("client connected to server OS port : %d, addr : %s\n", ntohs(clientaddr.sin_port), inet_ntoa(clientaddr.sin_addr)); 
    
    Rio_readinitb(&rio, clientfd);

    if (Fgets(buf, MAXLINE, stdin) != NULL) {
	transfered = 0;
	memcpy(filename, buf, strlen(buf)-1);
        Rio_writen(clientfd, buf, strlen(buf));
	FILE *fp = fopen(basename(filename), "w+");

	time(&start);	

        while((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0) {
		Rio_writen(fileno(fp), buf, n);
		transfered += n;
        }
	fclose(fp);
    }
    Close(clientfd);
    time(&stop);
    duration = difftime(stop, start);
    printf("Fin du transfert de fichier : %d octets transférés en %d secondes, vitesse :%d Kb/s\n",transfered, duration, (transfered/1000)/duration);
    exit(0);
}

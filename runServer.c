/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <stdio.h>
#include <stdlib.h>

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    long bufsize;
    Rio_readinitb(&rio, connfd);
    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes && contenu : %s\n", (unsigned int)n, buf);

	char *source = NULL;
	buf[strlen(buf)-1]='\0';
	FILE *fp = fopen(buf, "r");
	if (fp != NULL) {
	    if (fseek(fp, 0L, SEEK_END) == 0) {
		bufsize = ftell(fp);
		if (bufsize == -1) { /* Error */ }
		/* Allocate our buffer to that size. */
		source = malloc(sizeof(char) * (bufsize + 1));
		/* Go back to the start of the file. */
		if (fseek(fp, 0L, SEEK_SET) != 0) { printf("error"); }
		/* Read the entire file into memory. */
		size_t newLen = fread(source, sizeof(char), bufsize, fp);
		if ( ferror( fp ) != 0 ) {
		    fputs("Error reading file", stderr);
		} else {
		    source[newLen++] = '\0'; /* Just to be safe. */
		}
	    }
	    fclose(fp);
	} else {
		printf("ERROR FILE NOT FOUND");
	}
	Rio_writen(connfd, source, bufsize + 1);
	printf("ok\n");
	free(source); /* Don't forget to call free() later! */
    }
}


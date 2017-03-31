/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 1000000

void echo(int connfd)
{
    size_t size, bufContentSize;
    char buf[MAXLINE];
    unsigned char *bufContent;
    rio_t rio;
    int error = 0;
    Rio_readinitb(&rio, connfd);
    if ((size = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes && contenu : %s\n", (unsigned int)size, buf);
    	buf[strlen(buf)-1]='\0';
    	FILE *fp = fopen(buf, "r");
        if (fp != NULL){
            Rio_writen(connfd, "OK-", strlen("OK"));
            bufContent = (unsigned char*) malloc(BLOCK_SIZE);
            while ((bufContentSize = fread(bufContent, sizeof(unsigned char), BLOCK_SIZE, fp)) > 0) {
                if (!ferror(fp))
                    Rio_writen(connfd, bufContent, bufContentSize);
                else
                    Rio_writen(connfd, "AN ERROR OCCURED DURING THE FILE READING\n", strlen("AN ERROR OCCURED DURING THE FILE READING\n"));
            }
        } else
            error = 1;
        if (error){
            char *strerr = strerror(errno);
            Rio_writen(connfd, "KO", strlen("KO"));
            Rio_writen(connfd, strerr, strlen(strerr));
        }
        free(bufContent);
    }
}

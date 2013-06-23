/*
    Little Smalltalk, version 3
    Main Driver
    written By Tim Budd, September 1988
    Oregon State University
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "memory.h"
#include "names.h"
#include "interp.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

object firstProcess;
int initial = 0;    /* not making initial image */

#if defined TW_ENABLE_FFI
extern void initFFISymbols();   /* FFI symbols */
#endif

int main(int argc, char** argv)
{
    FILE *fp;
    char *p, buffer[255];

    int sockfd;
    struct protoent *protocol;
    int yes=1;
    //char yes='1'; // Solaris people use this
    int flags;
    struct sockaddr_in socketaddr;
    struct sockaddr_in newsocketaddr;
    int newsockfd;
    int res;
#ifndef WIN32
    socklen_t client_len;
#else
    int client_len;
#endif



    strcpy(buffer,"systemImage");
    p = buffer;

    if (argc != 1) p = argv[1];

    fp = fopen(p, "rb");

    if (fp == NULL) 
    {
        sysError("cannot open image", p);
        exit(1);
    }

    initialiseInterpreter();

    imageRead(fp);

    initCommonSymbols();
#if defined TW_ENABLE_FFI
    initFFISymbols();
#endif

    runCode("ObjectMemory changed: #returnFromSnapshot");
    firstProcess = globalSymbol("systemProcess");
    if (firstProcess == nilobj) 
    {
        sysError("no initial process","in image");
        exit(1); 
        return 1;
    }

    /* Initialise the dev socket */
    protocol = getprotobyname("tcp");
    sockfd = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

    /* lose the pesky "Address already in use" error message */
    /* \todo: Need to check return */
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)); 

#ifndef WIN32
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#else
    u_long iMode = 1;
    ioctlsocket(sockfd, FIONBIO, &iMode);
#endif
    /* Bind to the appropriate port */
    /* clear and initialize socketaddr */
    memset(&socketaddr, 0, sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_addr.s_addr = INADDR_ANY;
    socketaddr.sin_port = htons(8092);

    /* everything is setup, now we connect */
    /* \todo: Need to check return */
    bind(sockfd, (const struct sockaddr*)&socketaddr, sizeof(socketaddr));
    /* \todo: Need to check return */
    listen(sockfd, 0);





    /* execute the main system process loop repeatedly */
    /*debugging = TRUE;*/

    newsockfd = -1;

    while (execute(firstProcess, 15000)) 
    {
        errno = 0;

        if(newsockfd == -1)
        {
            /* clear and initialize client socketaddr */
            memset(&newsocketaddr, 0, sizeof(newsocketaddr));
            newsockfd = accept(sockfd, (struct sockaddr*)&newsocketaddr, &client_len);
            if(newsockfd >= 0)
                printf("Accepted a connection\n");
        }

        if(newsockfd >= 0)
        {
            /* Now read the stuff from the socket */
            errno = 0;
            res = read(newsockfd, buffer, 254);
            if(res > 0)
            {
                buffer[res] = '\0';
                printf("Read %s\n", buffer);
                runCode(buffer);
            }
            else
            {
                if(errno == 0)
                {
#ifndef WIN32
                    res = close(newsockfd);
#else
                    res = closesocket(newsockfd);
#endif
                    printf("Done with that connection\n");
                    newsockfd = -1;
                }
            }
        }
    }

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); 
    return 0;
}

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

object firstProcess;
int initial = 0;    /* not making initial image */

extern int objectCount();
extern noreturn initFFISymbols();   /* FFI symbols */

int main(int argc, char** argv)
{
    FILE *fp;
    char *p, buffer[120];

    initMemoryManager();
 
    strcpy(buffer,"systemImage");
    p = buffer;

# ifdef STDWIN
    /* initialize the standard windows package */
    winit();
    wmenusetdeflocal(1);
# endif

    if (argc != 1) p = argv[1];

# ifdef BINREADWRITE
    fp = fopen(p, "rb");
# endif
# ifndef BINREADWRITE
    fp = fopen(p, "r");
# endif

    if (fp == NULL) 
    {
        sysError("cannot open image", p);
        exit(1);
    }

    imageRead(fp);
    initCommonSymbols();
    initFFISymbols();

    firstProcess = globalSymbol("systemProcess");
    if (firstProcess == nilobj) 
    {
        sysError("no initial process","in image");
        exit(1); 
        return 1;
    }

    /* execute the main system process loop repeatedly */
    /*debugging = true;*/

# ifndef STDWIN
    /* not using windowing interface, safe to print out message */
    printf("Little Smalltalk, Version 3.04\n");
    printf("Written by Tim Budd, Oregon State University\n");
# endif

    while (execute(firstProcess, 15000)) ;

# ifdef STDWIN
    wdone();
# endif

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); 
    return 0;
}

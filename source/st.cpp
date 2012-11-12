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

ObjectHandle firstProcess;
int initial = 0;    /* not making initial image */

#if defined TW_ENABLE_FFI
extern void initFFISymbols();   /* FFI symbols */
#endif

int main(int argc, char** argv)
{
    FILE *fp;
    char *p, buffer[120];

    strcpy(buffer,"systemImage");
    p = buffer;

    if (argc != 1) p = argv[1];

    fp = fopen(p, "rb");

    if (fp == NULL) 
    {
        sysError("cannot open image", p);
        exit(1);
    }

    MemoryManager::Instance()->imageRead(fp);
    MemoryManager::Instance()->garbageCollect();

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

    /* execute the main system process loop repeatedly */
    /*debugging = true;*/

    while (execute(firstProcess, 15000)) ;

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); 
    return 0;
}

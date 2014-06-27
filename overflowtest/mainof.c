#define _GNU_SOURCE

#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "mybacktrace.h"

void recursiondeath()
{
    char buffer[1024];
    recursiondeath();
    memset(buffer, 0xeb, sizeof(buffer));
    printf("%d", buffer[0]);
}

int main(int argc, char const *argv[])
{
    char altstack[SIGSTKSZ];
    registersignal();
    addsignalstack(altstack, SIGSTKSZ);

    recursiondeath();

    return 0;
}
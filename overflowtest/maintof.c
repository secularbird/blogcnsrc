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

void* threadloop(void* args)
{
    // using sginal stack
    char altstack[SIGSTKSZ];
    addsignalstack(altstack, SIGSTKSZ);

    recursiondeath();
    return NULL;
}

int main(int argc, char const *argv[])
{
    char altstack[SIGSTKSZ];
    registersignal();
    addsignalstack(altstack, SIGSTKSZ);

    pthread_t t;
    pthread_create(&t, NULL, threadloop, NULL);

    pthread_join(t, NULL);

    return 0;
}
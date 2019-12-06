#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lockentry locks[NLOCKS];
int nextlock;
int locks_traverse;

/* Initialize the lock data structure on system startup */
void linit()
{
    locks_traverse = 0;
    nextlock = NLOCKS - 1;

    struct lockentry *lptr;
    int lockCount = 0, procCount = 0;
    int temp;

    while (lockCount < NLOCKS)
    {
        lptr = &locks[lockCount];
        lptr->lockState = LFREE;
        lptr->lockPriority = -1;

        /* Initializing the process log */
        procCount = 0;
        for (procCount = 0; procCount < NPROC; procCount++)
        {
            lptr->procLog[procCount] = 0;
        }

        /* Initilizing the lock queue head and tail */
        lptr->lockqueueHead = newqueue();
        temp = lptr->lockqueueHead;
        lptr->lockqueueTail = 1 + temp;

        lockCount++;
    }
}

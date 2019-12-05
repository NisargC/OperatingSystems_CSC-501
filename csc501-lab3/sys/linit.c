#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lock.h"

struct lockentry locks[NLOCKS];
int nextlock;
int locks_traverse;

/* Initialize the lock data structure on system startup */
void linit(){
    locks_traverse = 0;
    struct lockentry *lptr;
    int lockCount=0, procCount=0;
	int temp;
    nextlock = NLOCKS - 1;

    while(lockCount < NLOCKS){
        lptr = &locks[lockCount];
        lptr->lockState = LFREE;
        lptr->lockqueueHead = newqueue();
        temp = lptr->lockqueueHead;
	    lptr->lockPriority  = -1;	
        lptr->lockqueueTail = 1 + temp;

        procCount=0;
        for (procCount = 0; procCount<NPROC; procCount++){
            lptr->procLog[procCount] = 0;
        }

        lockCount++;
    }
}

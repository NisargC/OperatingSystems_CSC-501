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
    int outerCount=0, innerCount=0;
	int temp;
    nextlock = NLOCKS - 1;

    while(outerCount < NLOCKS){
        lptr = &locks[outerCount];
        lptr->lockState = LFREE;
        lptr->lockqueueHead = newqueue();
        temp = lptr->lockqueueHead;
	    lptr->lockPriority  = -1;	
        lptr->lockqueueTail = 1 + temp;
        innerCount=0;
        for (innerCount = 0; innerCount<NPROC; innerCount++){
            lptr->proc_log[innerCount] = 0;
        }
        outerCount++;
    }
}

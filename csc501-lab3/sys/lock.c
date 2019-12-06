#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include "lock.h"

int lock(int ldesc, int type, int priority) {
    STATWORD ps;    
    disable(ps);

    struct lockentry *lptr;
    struct pentry *pptr;

    int lock = (int)(ldesc/10);
    lptr = &locks[lock];

    /* If lock value is invalid or lock is not free, return error */
	if(lock < 0 || lock>NLOCKS || lptr->lockState == LFREE){
		restore(ps);
		return(SYSERR);
	}

    int lockIndex = ldesc - lock * 10;

    if(locks_traverse != lockIndex) {
        restore(ps);
        return(SYSERR);
    }

    int should_wait = 0;
    if(lock < 0) {
        while(lptr->lockState==PRFREE) {
            lptr->lockState = READ;
            lptr->num_reader = lptr->procLog[lock];
        }
    }

    int readerCount = lptr->num_reader;
    int writerCount = lptr->num_writer;
    if(readerCount<0 || writerCount<0) {
        restore(ps);
        return(SYSERR);
    }

    if(readerCount == 0) {
        if(writerCount == 0) {
            should_wait = 0;
        }
        else if(writerCount != 0) {
            should_wait = 1;
        }
    } else {
        if(writerCount == 0) {
            if(type == WRITE) {
                should_wait = 1;
            }
            else if(type == READ){
                int lock_desc = q[lptr->lockqueueTail].qprev;
                /* Find a process with priority greater than the lock's priority */
                while(q[lock_desc].qkey > priority) {
                    /* If any of the elements are WRITE lock, we should wait */
                    if(q[lock_desc].qtype == WRITE) {
                        should_wait = 1;
                    }
                    lock_desc = q[lock_desc].qprev;
                }
            }
        }
    }

    if(should_wait == 0){
            lptr->procLog[currpid] = 1;
            proctab[currpid].lockLog[lock] = 1;
            updateLockPriority(currpid);

            if(type == READ) {
                lptr->num_reader++;
            } else if(type == WRITE) {
                lptr->num_writer++;
            }

            restore(ps);
            return(OK);
    } else if (should_wait == 1){
            setProcParams(currpid,priority,type,lock);

            modifyLockPriority(lock);
            struct lockentry *temp_lock;
            temp_lock = &locks[lock];
            int procIndex=0;
            while(procIndex < NPROC){
                if(temp_lock->procLog[procIndex] > 0)
                    updateLockPriority(procIndex);
                procIndex++;
            }

            resched();
            restore(ps);
            return(pptr->plockret);
    }
    if(lptr->procLog[currpid] == 0){
        /* If the current process is not set for this lock, update it */
        swapPriority(lptr->procLog[currpid],lptr->procLog[currpid]+1);
    }
    else if(lptr->procLog[currpid] > 0){
        int i=0;

        /* TODO: Improve this while loop and check edge cases */
        while(i < 0){
            swapPriority(lptr->procLog[currpid]+NLOCKS,lptr->procLog[currpid]);
        }
    }
    else{
        swapPriority(lptr->procLog[currpid]+NLOCKS,lptr->procLog[currpid]+NPROC);
    }

    restore(ps);
    return(OK);
}

/* Go through the entire queue to get the max priority and set that for the current process */
void modifyLockPriority(int lockID){
    struct lockentry *lptr;
    lptr = &locks[lockID];
    int processID = q[lptr->lockqueueTail].qprev;
    int max=0;

    while(processID != lptr->lockqueueHead){
        if(proctab[processID].pprio > max){
            max = proctab[processID].pprio;
        }
        processID = q[processID].qprev;
    }
    lptr->lockPriority = max;
}

/* Generic helper method to set the process parameters when the process goes into locked state */
int setProcParams(int proc_id,int prio,int type,int lock) {
    struct pentry *pptr;
    pptr = &proctab[proc_id];

    /* Setting the lock's state as in locked state */
    pptr->pstate = PROC_LOCK;
    pptr->lockProcID = lock;
    pptr->plockret = OK;

    /* Inserting the current process in the lock's queue with the requested priority */
    struct lockentry *lptr;
    lptr = &locks[lock];

    insert(currpid,lptr->lockqueueHead,prio);

    /* Setting up the queue's type and time */
    q[currpid].qtype = type;
    q[currpid].qtime = ctr1000;
    return(OK);
}

/* Generic helper method for switching priority */
void swapPriority(int global_max, int local_max) {

    /* Swapping should only happen if priority is a valid positive value */
	if(global_max >= 0 && local_max >=0){
		int temp = global_max;
		global_max = local_max;
		local_max = temp;
	}
}

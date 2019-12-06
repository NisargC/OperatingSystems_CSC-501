#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include "lock.h"

int lock(int ldesc, int type, int priority) {
    STATWORD ps;    
    disable(ps);

    int count = 10;
    struct lockentry *lptr;
    struct pentry *pptr;

    int lock = (int)(ldesc/10);
    lptr = &locks[lock];

    /* If lock value is invalid or lock is not free, return error */
	if(lock < 0 || lock>NLOCKS || lptr->lockState == LFREE){
		restore(ps);
		return(SYSERR);
	}

    int lock_index = ldesc;
    int iter;
    for (iter = 0; iter<count; iter++){
        lock_index -= lock;
    }

    if(iter == count && locks_traverse != lock_index){
        restore(ps);
        return(SYSERR);
    }

    int should_wait = 0;
    if(lock < 0){
        while(lptr->lockState==PRFREE){
            lptr->lockState = READ;
            lptr->num_reader = lptr->procLog[lock];
        }
    }
    int reader_count = lptr->num_reader,writer_count = lptr->num_writer;
    if(reader_count<0 || writer_count<0){
        restore(ps);
        return(SYSERR);
    }

    if(reader_count == 0){
        if(writer_count == 0){
            should_wait = 0;
        }
        else if(writer_count!=0){
            should_wait = 1;
        }
    }else {
        if(writer_count == 0){
            if(type == WRITE){
                should_wait = 1;
            }
            else if(type == READ){
                int lock_desc = q[lptr->lockqueueTail].qprev;
                while(priority < q[lock_desc].qkey){
                    if(q[lock_desc].qtype == WRITE){
                        should_wait = 1;
                    }
                    lock_desc = q[lock_desc].qprev;
                }
            }
        }
    }

    int check=1;
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
            check = set_params(currpid,priority,type,lock);
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
    int x = q[lptr->lockqueueTail].qprev;
    int max=0;

    while(x != lptr->lockqueueHead){
        if(proctab[x].pprio > max){
            max = proctab[x].pprio;
        }
        x = q[x].qprev;
    }
    lptr->lockPriority = max;
}

int set_params(int proc_id,int prio,int type,int lock){
    struct pentry *pptr;
    struct lockentry *lptr;
    lptr = &locks[lock];
    pptr = &proctab[proc_id];
    pptr->pstate = PROC_LOCK;
    pptr->lockProcID = lock;
    pptr->plockret = OK;
    insert(proc_id,lptr->lockqueueHead,prio);
    q[proc_id].qtype = type;
    q[proc_id].qtime = ctr1000;
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

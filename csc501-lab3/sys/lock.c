#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include "lock.h"

int lock(int ldes1, int type, int priority) {
    STATWORD ps;    
    disable(ps);

    int count = 10;
    struct lockentry *lptr;
    struct pentry *pptr;
    
    int lock = (int)(ldes1/10);
    lptr = &locks[lock];
	if(lock < 0 || lock>NLOCKS || lptr->lockState == LFREE){
		restore(ps);
		return(SYSERR);
	}

    int lock_index = ldes1;
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
            lptr->num_reader = lptr->proc_log[lock];
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
            lptr->proc_log[currpid] = 1;
            proctab[currpid].lock_log[lock] = 1;
            updateLockPriority(currpid);
            switch(type){
                case READ:
                    lptr->num_reader++;
                    break;
                case WRITE:
                    lptr->num_writer++;
                    break;
                default:
                    break;
            }
            restore(ps);
            return(OK);
    } else if (should_wait == 1){
            check = set_params(currpid,priority,type,lock);
            modifyLockPriority(lock);
            struct lockentry *temp_lock;
            temp_lock = &locks[lock];
            int i=0;
            while(i < NPROC){
                if(temp_lock->proc_log[i] > 0)
                    updateLockPriority(i);
                i++;
            }

            resched();
            restore(ps);
            return(pptr->plockret);
    }
    if(lptr->proc_log[currpid] == 0){
        swapPriority(lptr->proc_log[currpid],lptr->proc_log[currpid]+1);
    }
    else if(lptr->proc_log[currpid] > 0){
        int i=0;
        while(i < 0){
            swapPriority(lptr->proc_log[currpid]+NLOCKS,lptr->proc_log[currpid]);
        }
    }
    else{
        swapPriority(lptr->proc_log[currpid]+NLOCKS,lptr->proc_log[currpid]+NPROC);
    }

    restore(ps);
    return(OK);
}

/* Go through the entire queue to get the max priority and set that for the current process */
void modifyLockPriority(int l_id){
    struct lockentry *lptr;
    lptr = &locks[l_id];
    int x = q[lptr->lockqueueTail].qprev,max=0;

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
    pptr->l_proc = lock;
    pptr->plockret = OK;
    insert(proc_id,lptr->lockqueueHead,prio);
    q[proc_id].qtype = type;
    q[proc_id].qtime = ctr1000;
    return(OK);
}

void releaseWriter(int l_id, int p) {
    struct lockentry *lptr;
    lptr = &locks[l_id];
    lptr->proc_log[p] = 1;
    proctab[currpid].lock_log[l_id] = 1;
    if(q[p].qtype == READ) {
        lptr->num_reader += 1;
    } else if (q[p].qtype == WRITE) {
        lptr->num_writer += 1;
    }

    modifyLockPriority(l_id);
    int i;
    for(i=0; i < NPROC;i++){
        if(lptr->proc_log[i] > 0)
            updateLockPriority(i);
    }
    dequeue(p);
    ready(p, RESCHNO);
}

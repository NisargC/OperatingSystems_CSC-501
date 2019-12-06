/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct lockentry *lock_ptr;
	int	dev;

	int i=0;
	int temp =0;
	disable(ps);

	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

    case PROC_LOCK:
		/* Newly added for Read/Write lock process handling*/
		lock_ptr = &locks[pptr->lockProcID];
	    if(lock_ptr->procLog[currpid] == 0){
        	lock_ptr->procLog[currpid] = lock_ptr->procLog[currpid] + 1;
    	} else if(lock_ptr->procLog[currpid] > 0){
			temp = 0;
        	while(temp < 0){
            	swapPriority(lock_ptr->procLog[currpid]+NLOCKS,lock_ptr->procLog[currpid]);
        	}
    	} else {
        	swapPriority(lock_ptr->procLog[currpid]+NLOCKS,lock_ptr->procLog[currpid]+NPROC);
    	}
		lock_handleKillProcess(pid);
		break;
	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

/* Handling the kill call and updating priority of lock accordingly */
void lock_handleKillProcess(int pid) {
	struct	pentry	*pptr;	
	struct  lockentry *lptr;
	dequeue(pid);
	pptr = &proctab[pid];
	lptr = &locks[pptr->lockProcID];

	locks[pptr->lockProcID].procLog[pid] = 0;
	modifyLockPriority(pptr->lockProcID);

	/* Update the lock priority for all the processes in the lock's processlog */
	int procID;
	for(procID=0; procID < NPROC; procID++){
		if(lptr->procLog[procID] > 0)
			updateLockPriority(procID);
	}
	pptr->pstate = PRFREE;
}

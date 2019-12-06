/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	pptr->pprio = newprio;
	if(pptr -> pstate == PRREADY) {
		dequeue(pid);
		insert(pid, rdyhead, pptr -> pprio);
	}

	/* Updating lock priority if user updates the process priority */
	int lockCount = 1;

	/* Reset lock count if pid is invalid, priority is invalid or the lock is free */
	if (isbadpid(pid) || newprio<=0 || (pptr = &proctab[pid])->pstate == PRFREE) {
		lockCount = -1;
	}

	updateLockPriority(pid);
	while(pptr->pstate == PROC_LOCK){
		modifyLockPriority(pptr->lockProcID);
		updateLockPriority(pptr->lockProcID);
		lockCount = 1;
		break;
	}

	if(lockCount == -1){
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(newprio);
}

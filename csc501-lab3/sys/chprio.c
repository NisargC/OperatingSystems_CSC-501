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

	int flag = updatePriority(pid, newprio);
	if(flag == -1){
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(newprio);
}

/* Updating lock priority if user updates the process priority */
int updatePriority(int pid, int newprio) {
	int lockCount = 1;
	struct	pentry	*pptr;

	/* Reset lock count if pid is invalid, priority is invalid or the lock is free */
	if (isbadpid(pid) || newprio<=0 || (pptr = &proctab[pid])->pstate == PRFREE) {
		lockCount = -1;
	}
	updateLockPriority(pid);
	while(pptr->pstate == PROC_LOCK){
		modifyLockPriority(pptr->l_proc);
		updateLockPriority(pptr->l_proc);
		lockCount = 1;
		break;
	}
	return(lockCount);
}

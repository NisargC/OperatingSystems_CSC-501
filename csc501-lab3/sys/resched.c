/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	optr = &proctab[currpid];

	/* If process queue has no other process and current is running, let it continue */
	if(optr->pstate == PRCURR && isempty(rdyhead)){
		return OK;
	}
	int pid = q[rdytail].qprev;
	
	/* PID of process with max priority */
	int maxPriority = -1,maxPID=-1;
	int tempPrio=0;

	/* Calculate the max priority from the queue */
	while(pid != rdyhead){
		maxPriority = tempPrio;
		maxPID = pid;
		pid = q[pid].qprev;
	}

	pid = q[rdytail].qprev;
	while(pid != rdyhead){
		if(proctab[pid].pinh == 0){
			tempPrio = proctab[pid].pprio;
		} else {
			tempPrio = proctab[pid].pinh;
		}
		if(tempPrio > maxPriority){
			maxPriority = tempPrio;
			maxPID = pid;
		}
		pid = q[pid].qprev;
	}

	/* If the inhertited priority of the old process is 0, check if it's priority is the max */
	if(optr->pstate == PRCURR) {
		if(optr->pinh == 0) {
			if(maxPID < optr->pprio)
				return OK;
		} else {
			if(maxPID < optr->pinh)
				return OK;
		}
	}
	
	/* no switch needed if current process priority higher than next */
	if (((optr= &proctab[currpid])->pstate == PRCURR) && (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	
	currpid = maxPID;
    dequeue(maxPID);
	nptr = &proctab[currpid];
	//nptr = &proctab[ (currpid = getlast(rdytail)) ];

	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

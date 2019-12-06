/* sreset.c - sreset */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  sreset  --  reset the count and queue of a semaphore
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL sreset(int sem, int count)
{
	STATWORD ps;    
	struct	sentry	*sptr;
	int	pid;
	int	slist;

	disable(ps);
	if (isbadsem(sem) || count<0 || semaph[sem].sstate==SFREE) {
		restore(ps);
        	if(start_stop_flag==1){
	                struct pentry *proc = &proctab[currpid];
        	        proc->end_time_array[22] += ctr1000;
	        }
		return(SYSERR);
	}
	sptr = &semaph[sem];
	slist = sptr->sqhead;
	while ((pid=getfirst(slist)) != EMPTY)
		ready(pid,RESCHNO);
	sptr->semcnt = count;
	resched();
	restore(ps);

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[22] += ctr1000;
        }
	return(OK);
}

/* suspend.c - suspend */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  suspend  --  suspend a process, placing it in hibernation
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	suspend(int pid)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[24]=proc->call_count_array[24]+1;
                 proc->start_time_array[24] += ctr1000;
        }
	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority returned		*/

	disable(ps);
	if (isbadpid(pid) || pid==NULLPROC ||
	 ((pptr= &proctab[pid])->pstate!=PRCURR && pptr->pstate!=PRREADY)) {
		restore(ps);
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
        	        proc->end_time_array[24] += ctr1000;
	        }
		return(SYSERR);
	}
	if (pptr->pstate == PRREADY) {
		pptr->pstate = PRSUSP;
		dequeue(pid);
	}
	else {
		pptr->pstate = PRSUSP;
		resched();
	}
	prio = pptr->pprio;
	restore(ps);
        
	if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[24] += ctr1000;
        }
	return(prio);
}

/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL chprio(int pid, int newprio)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[1]=proc->call_count_array[1]+1;
                 proc->start_time_array[1] += ctr1000;
        }

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
        	if(start_stop_flag==1){
                	struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[1] += ctr1000;
        	}
		return(SYSERR);
	}
	pptr->pprio = newprio;
	restore(ps);
        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[1] += ctr1000;
        }
	return(newprio);
}

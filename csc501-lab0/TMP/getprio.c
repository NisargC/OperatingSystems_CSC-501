/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL getprio(int pid)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[3]=proc->call_count_array[3]+1;
                 proc->start_time_array[3] += ctr1000;
        }

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
        	if(start_stop_flag==1){
                	struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[3] += ctr1000;
        	}
		return(SYSERR);
	}
	restore(ps);
        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[3] += ctr1000;
        }
	return(pptr->pprio);
}

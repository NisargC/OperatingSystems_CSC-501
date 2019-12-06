/* unsleep.c - unsleep */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * unsleep  --  remove  process from the sleep queue prematurely
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	unsleep(int pid)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[25]=proc->call_count_array[25]+1;
                 proc->start_time_array[25] += ctr1000;
        }
	STATWORD ps;    
	struct	pentry	*pptr;
	struct	qent	*qptr;
	int	remain;
	int	next;

        disable(ps);
	if (isbadpid(pid) ||
	    ( (pptr = &proctab[pid])->pstate != PRSLEEP &&
	     pptr->pstate != PRTRECV) ) {
		restore(ps);
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[25] += ctr1000;
        	}
		return(SYSERR);
	}
	qptr = &q[pid];
	remain = qptr->qkey;
	if ( (next=qptr->qnext) < NPROC)
		q[next].qkey += remain;
	dequeue(pid);
	if ( (next=q[clockq].qnext) < NPROC)
		sltop = (int *) & q[next].qkey;
	else
		slnempty = FALSE;
        restore(ps);

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[25] += ctr1000;
        }
	return(OK);
}

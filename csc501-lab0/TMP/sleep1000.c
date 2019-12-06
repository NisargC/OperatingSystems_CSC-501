/* sleep1000.c - sleep1000 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * sleep1000 --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL sleep1000(int n)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[21]=proc->call_count_array[21]+1;
                 proc->start_time_array[21] += ctr1000;
        }
	STATWORD ps;    

	if (n < 0  || clkruns==0){
	        if(start_stop_flag==1){
               		struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[21] += ctr1000;
        	}
        	return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep1000(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[21] += ctr1000;
        }
	return(OK);
}

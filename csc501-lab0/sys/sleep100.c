/* sleep100.c - sleep100 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * sleep100  --  delay the caller for a time specified in 1/100 of seconds
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL sleep100(int n)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[20]=proc->call_count_array[20]+1;
                 proc->start_time_array[20] += ctr1000;
        }
	STATWORD ps;    

	if (n < 0  || clkruns==0){
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
        	        proc->end_time_array[20] += ctr1000;
	        }
        	return(SYSERR);
	}
	disable(ps);
	if (n == 0) {		/* sleep100(0) -> end time slice */
	        ;
	} else {
		insertd(currpid,clockq,n*10);
		slnempty = TRUE;
		sltop = &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
        restore(ps);
        
	if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[20] += ctr1000;
        }
	return(OK);
}

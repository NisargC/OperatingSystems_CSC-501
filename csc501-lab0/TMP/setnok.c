/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	setnok(int nok, int pid)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[14]=proc->call_count_array[14]+1;
                 proc->start_time_array[14] += ctr1000;
        }
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
        
		if(start_stop_flag==1){
                	struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[14] += ctr1000;
        	}
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);
        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[14] += ctr1000;
        }
	return(OK);
}

/* wait.c - wait */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * wait  --  make current process wait on a semaphore
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	wait(int sem)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[26]=proc->call_count_array[26]+1;
                 proc->start_time_array[26] += ctr1000;
        }
	STATWORD ps;    
	struct	sentry	*sptr;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[26] += ctr1000;
        	}
		return(SYSERR);
	}
	
	if (--(sptr->semcnt) < 0) {
		(pptr = &proctab[currpid])->pstate = PRWAIT;
		pptr->psem = sem;
		enqueue(currpid,sptr->sqtail);
		pptr->pwaitret = OK;
		resched();
		restore(ps);
	
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[26] += ctr1000;
	        }
		return pptr->pwaitret;
	}
	restore(ps);

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[26] += ctr1000;
        }
	return(OK);
}

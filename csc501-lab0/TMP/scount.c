/* scount.c - scount */

#include <conf.h>
#include <kernel.h>
#include <sem.h>
#include <proc.h>
/*------------------------------------------------------------------------
 *  scount  --  return a semaphore count
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL scount(int sem)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[10]=proc->call_count_array[10]+1;
                 proc->start_time_array[10] += ctr1000;
        }
extern	struct	sentry	semaph[];

	if (isbadsem(sem) || semaph[sem].sstate==SFREE){
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[10] += ctr1000;
        	}
		return(SYSERR);
}
        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[10] += ctr1000;
        }
	return(semaph[sem].semcnt);
}

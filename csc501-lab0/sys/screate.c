/* screate.c - screate, newsem */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

LOCAL int newsem();

/*------------------------------------------------------------------------
 * screate  --  create and initialize a semaphore, returning its id
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL screate(int count)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[15]=proc->call_count_array[15]+1;
                 proc->start_time_array[15] += ctr1000;
        }
	STATWORD ps;    
	int	sem;

	disable(ps);
	if ( count<0 || (sem=newsem())==SYSERR ) {
		restore(ps);
	        if(start_stop_flag==1){
        	        struct pentry *proc = &proctab[currpid];
                	proc->end_time_array[15] += ctr1000;
	        }
		return(SYSERR);
	}
	semaph[sem].semcnt = count;
	/* sqhead and sqtail were initialized at system startup */
	restore(ps);
        
	if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[15] += ctr1000;
        }
	return(sem);
}

/*------------------------------------------------------------------------
 * newsem  --  allocate an unused semaphore and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newsem()
{
	int	sem;
	int	i;

	for (i=0 ; i<NSEM ; i++) {
		sem=nextsem--;
		if (nextsem < 0)
			nextsem = NSEM-1;
		if (semaph[sem].sstate==SFREE) {
			semaph[sem].sstate = SUSED;
			return(sem);
		}
	}
	return(SYSERR);
}

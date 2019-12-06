/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>
extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	gettime(long *timvar)
{
	if(start_stop_flag==1){
        	struct pentry *proc = &proctab[currpid];
                proc->call_count_array[4]=proc->call_count_array[4]+1;
                proc->start_time_array[4] += ctr1000;
        }
    /* long	now; */

	/* FIXME -- no getutim */

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[4] += ctr1000;
        }

    return OK;
}

/* setdev.c - setdev */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

/*------------------------------------------------------------------------
 *  setdev  -  set the two device entries in the process table entry
 *------------------------------------------------------------------------
 */
extern int start_stop_flag;
long ctr1000;

SYSCALL	setdev(int pid, int dev1, int dev2)
{
        if(start_stop_flag==1){
                 struct pentry *proc = &proctab[currpid];
                 proc->call_count_array[13]=proc->call_count_array[13]+1;
                 proc->start_time_array[13] += ctr1000;
        }
	short	*nxtdev;

	if (isbadpid(pid))
		return(SYSERR);
	nxtdev = (short *) proctab[pid].pdevs;
	*nxtdev++ = dev1;
	*nxtdev = dev2;

        if(start_stop_flag==1){
                struct pentry *proc = &proctab[currpid];
                proc->end_time_array[13] += ctr1000;
        }
	return(OK);
}

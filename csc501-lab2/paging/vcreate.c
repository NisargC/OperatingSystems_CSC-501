/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	disable(ps);

	int pid;
	pid = create(procaddr,ssize,priority,name,nargs,args);
	int pointer_backing_store;
	if(get_bsm(&pointer_backing_store) == SYSERR) {
		return SYSERR;
	}

	struct mblock *base;
	base = BACKING_STORE_BASE + (pointer_backing_store * BACKING_STORE_UNIT_SIZE);
	base->mlen = hsize * NBPG;
	base->mnext = NULL;

	proctab[pid].store = pointer_backing_store;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vhpno = BS_VIRTUAL_BASE_PAGE;
	proctab[pid].vmemlist->mnext = BS_VIRTUAL_BASE_PAGE * NBPG;

	bsm_tab[pointer_backing_store].bs_status = BSM_MAPPED;
	bsm_tab[pointer_backing_store].bs_pid = pid;
	bsm_tab[pointer_backing_store].bs_vpno = BS_VIRTUAL_BASE_PAGE;
	bsm_tab[pointer_backing_store].bs_npages = hsize;
	bsm_tab[pointer_backing_store].bs_heap_private = 1;
	bsm_tab[pointer_backing_store].bs_sem = 0;

	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}

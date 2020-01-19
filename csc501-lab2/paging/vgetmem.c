/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{

	STATWORD ps;
	disable(ps);

	struct mblock *virtual_memlist_pointer;
	struct mblock *next_pointer;
	struct mblock *current_pointer;
	struct mblock *rem_pointer;

	virtual_memlist_pointer=proctab[currpid].vmemlist;

	if(virtual_memlist_pointer->mnext == (struct mblock *)NULL || nbytes == 0) {
		restore(ps);
		kprintf("Syserr in vgetmem memlist null");
		return ((WORD *)SYSERR);
	}

	/* https://stackoverflow.com/questions/52883965/xinu-os-understanding-roundmb-function */
	nbytes = roundmb(nbytes);

	next_pointer = virtual_memlist_pointer->mnext;
	current_pointer = virtual_memlist_pointer;

	while(next_pointer != NULL) {
		/* Check once why next */
		if(next_pointer->mlen == nbytes) {
			current_pointer->mnext = next_pointer->mnext;
			restore(ps);
			return ((WORD *)next_pointer);
		}
		if(next_pointer->mlen>nbytes) {
			rem_pointer = (struct mblock *)((unsigned)next_pointer+nbytes);
			rem_pointer->mlen = next_pointer->mlen-nbytes;
			rem_pointer->mnext = next_pointer->mnext;

			current_pointer->mnext = rem_pointer;

			restore(ps);
			return ((WORD *)next_pointer);
		}
		current_pointer = next_pointer;
		next_pointer = next_pointer->mnext;
	}

	restore(ps);
	kprintf("Syserr in vgetmem memlist exits while");
	return ((WORD *)SYSERR);
}



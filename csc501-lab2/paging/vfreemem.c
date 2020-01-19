/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);
	if (size == 0 || block < BS_VIRTUAL_BASE_PAGE * NBPG) {
		restore(ps);
		kprintf("Syserr in vfreemem size error");
		return SYSERR;
	}
	struct mblock *mem_list_pointer;
	struct mblock *prev_pointer;
	struct mblock *next_pointer;

	mem_list_pointer = proctab[currpid].vmemlist;
	prev_pointer = mem_list_pointer;
	next_pointer = prev_pointer->mnext;

	while (next_pointer != NULL && next_pointer < block) {
		prev_pointer = next_pointer;
		next_pointer = next_pointer->mnext;
	}

	block->mnext = next_pointer;
	prev_pointer->mnext = block;

	block->mlen = size;

	restore(ps);
	return OK;
}

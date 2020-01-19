/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm() {
	STATWORD ps;
	disable(ps);

	int count = 0;
	while(count < MAX_BS) {
		bsm_tab[count].bs_status = BSM_UNMAPPED;
		bsm_tab[count].bs_pid = -1;
		bsm_tab[count].bs_vpno = VPN_BASE;
		bsm_tab[count].bs_npages = 0;
		bsm_tab[count].bs_sem = 0;
		bsm_tab[count].bs_heap_private = 0;
		count++;
	}

	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail) {
	STATWORD ps;
	disable(ps);

	if(avail != NULL) {
	int count = 0;
		while(count < MAX_BS) {
			if(bsm_tab[count].bs_status == BSM_UNMAPPED){
				*avail = count;
				restore(ps);
				return OK;		
			}
			count = count + 1;
		}
	}
	restore(ps);
	kprintf("Cannot allocate the backing store, returning SYSERR\n");
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i) {
	// TODO: Find the use of this method
	STATWORD ps;
	disable(ps);
	
	bsm_tab[i].bs_status = BSM_UNMAPPED;
	bsm_tab[i].bs_npages = 0;
  	bsm_tab[i].bs_pid = -1;
	
	restore(ps);
	return OK;
}


/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth) {
	STATWORD ps;
	disable(ps);
	
	int count = 0;
	while(count < MAX_BS) {
		if(bsm_tab[count].bs_pid == pid) {
			/* Updating the values of pageth and store as the page is found */
			kprintf("In bsm_lookup in if in while");
			*pageth = (vaddr/NBPG) - bsm_tab[count].bs_vpno;
			*store = count;
			restore(ps);
			return OK;
		}
		count = count + 1;
	}

	kprintf("Could not find the requested backing store, returning SYSERR.");
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages) {

	/* Updating the starting pageno and the backing store for vheap */
	proctab[currpid].vhpno = vpno;
	proctab[currpid].store = source;

	/* Updating the mapping based on input arguments to the bsm_tab array */
	bsm_tab[source].bs_vpno = vpno;
	bsm_tab[source].bs_status = BSM_MAPPED;
	bsm_tab[source].bs_heap_private = 0;
	bsm_tab[source].bs_pid = pid;
	bsm_tab[source].bs_npages = npages;
	bsm_tab[source].bs_sem = 1;

	return OK;

}


/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag) {
	unsigned long virt_addr = vpno * NBPG;
	int backing_store_num;
	int pageth;

	int count = 0;
	while(count < NFRAMES){
		if(frm_tab[count].fr_pid == pid && frm_tab[count].fr_type == FR_PAGE) {
			/* Fetching the page using lookup and updating it */
			bsm_lookup(pid, virt_addr, &backing_store_num, &pageth);
			write_bs((count+NFRAMES)*NBPG, backing_store_num, pageth);
			kprintf("In bsm_unmap in if in while");
  		}
		count = count + 1;
	}

	kprintf("In bsm_unmap after while");

	/* Unmapping the Backing store */
	bsm_tab[backing_store_num].bs_vpno = VPN_BASE;
	bsm_tab[backing_store_num].bs_status = BSM_UNMAPPED;
	bsm_tab[backing_store_num].bs_heap_private = 0;
	bsm_tab[backing_store_num].bs_pid = -1;
	bsm_tab[backing_store_num].bs_npages = 0;
	bsm_tab[backing_store_num].bs_sem = 0;

	return OK;
}

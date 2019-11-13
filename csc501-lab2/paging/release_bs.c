#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
  	STATWORD ps;
	disable(ps);

    if(bs_id < 0 || bs_id > MAX_BS) {
        /* Invalid bs_id */
        restore(ps);
        return SYSERR;
    }

    bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
	bsm_tab[bs_id].bs_pid = -1;
	bsm_tab[bs_id].bs_vpno = BS_VIRTUAL_BASE_PAGE;
	bsm_tab[bs_id].bs_sem = 0;
	bsm_tab[bs_id].bs_npages = 0;
	bsm_tab[bs_id].bs_heap_private = 0;
	
    restore(ps);
	return OK;
}


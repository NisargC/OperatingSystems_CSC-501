#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

	STATWORD ps;
	disable(ps);

	if (bs_id < 0 || bs_id > MAX_BS) {
		/* Input arguments error */
		restore(ps);
		kprintf("Syserr in get_bs arg error bs_id");
		return SYSERR;
	}
	if (npages < 0 || npages > 256) {
		restore(ps);
		kprintf("Syserr in get_bs arg error npages");
		return SYSERR;
	}

	if(bsm_tab[bs_id].bs_status == BSM_MAPPED) {
		if (bsm_tab[bs_id].bs_sem == 1 || bsm_tab[bs_id].bs_heap_private == 1) {
			restore(ps);
			kprintf("Syserr in get_bs arg error private heap");
			return SYSERR;
		}
		else {
			bsm_tab[bs_id].bs_pid = currpid;
			restore(ps);
			return bsm_tab[bs_id].bs_npages;
		}	
	}
	else {
		bsm_tab[bs_id].bs_pid = currpid;
		bsm_tab[bs_id].bs_npages = npages;
		bsm_tab[bs_id].bs_status = BSM_MAPPED;
	}

	restore(ps);
    return npages;
}



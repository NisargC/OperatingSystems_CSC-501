/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
extern int set_page_table();
extern int page_replace_policy;

SYSCALL pfint()
{
    STATWORD ps;
  	disable(ps);

	unsigned long requested_virtual_address;
	virt_addr_t *virtual_address_struct;
	unsigned long pdbr;
	unsigned int page_offset;
    unsigned int pagetable_offset;
    unsigned int pagedirectory_offset;

	pd_t *pagedirectory_entry;
	pt_t *pagetable_entry;

	int pagetable_frame_index; 
	int empty_frame_index;

	requested_virtual_address = read_cr2();

	virtual_address_struct = (virt_addr_t*)&requested_virtual_address;

	pagedirectory_offset = virtual_address_struct->pd_offset;
	page_offset = virtual_address_struct->pg_offset;
	pagetable_offset = virtual_address_struct->pt_offset;

	pdbr = proctab[currpid].pdbr;

	pagedirectory_entry = pdbr + pagedirectory_offset * sizeof(pd_t);
	pagetable_entry = (pt_t*)(pagedirectory_entry->pd_base * NBPG + pagetable_offset * sizeof(pt_t));

	kprintf("PT_Offset %d",pagetable_offset);
	kprintf("PD_Offset %d",pagedirectory_offset);
	kprintf("PDBR %d",pdbr);

	if(pagedirectory_entry->pd_pres == 0) {
		pagetable_frame_index = set_page_table();

		pagedirectory_entry->pd_pres = 1;
		pagedirectory_entry->pd_avail = 0;
		pagedirectory_entry->pd_write = 1;
		pagedirectory_entry->pd_mbz = 0;
		pagedirectory_entry->pd_pcd = 0;
		pagedirectory_entry->pd_fmb = 0;
		pagedirectory_entry->pd_user = 1;
		pagedirectory_entry->pd_acc = 0;
		pagedirectory_entry->pd_global = 0;
		pagedirectory_entry->pd_pwt = 0;
		pagedirectory_entry->pd_base = pagetable_frame_index + FRAME0;

		frm_tab[pagetable_frame_index].fr_pid = currpid;
		frm_tab[pagetable_frame_index].fr_status = FRM_MAPPED;
		frm_tab[pagetable_frame_index].fr_type = FR_TBL;
	}

	int bs_reference;
	int bs_page_offset;

	if(pagetable_entry->pt_pres == 0) {
		get_frm(&empty_frame_index);

		pagetable_entry->pt_pres = 1;
		pagetable_entry->pt_base = (FRAME0 + empty_frame_index);
		pagetable_entry->pt_user = 1;
		pagetable_entry->pt_write = 1;

		frm_tab[pagedirectory_entry->pd_base-FRAME0].fr_refcnt++;
		frm_tab[empty_frame_index].fr_pid = currpid;
		frm_tab[empty_frame_index].fr_status = FRM_MAPPED;
		frm_tab[empty_frame_index].fr_dirty=0;
		frm_tab[empty_frame_index].fr_type = FR_PAGE;
		frm_tab[empty_frame_index].fr_vpno = requested_virtual_address/NBPG;
		
		bsm_lookup(currpid,requested_virtual_address,&bs_reference,&bs_page_offset);
		read_bs((char*)((FRAME0+empty_frame_index)*NBPG),bs_reference,bs_page_offset);

		insert_frame_SC_AGING(empty_frame_index);

	}

	write_cr3(pdbr);

	restore(ps);
	return OK;
}

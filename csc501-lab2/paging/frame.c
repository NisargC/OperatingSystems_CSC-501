/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

extern int page_replace_policy;
void page_directory_allocation(int);
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	STATWORD ps;
	disable(ps);
	
	int count = 0;
	while(count < NFRAMES) {
		frm_tab[count].fr_status = FRM_UNMAPPED;
		frm_tab[count].fr_pid = -1;
		frm_tab[count].fr_type = FR_PAGE;
		frm_tab[count].fr_refcnt = 0;
		frm_tab[count].fr_vpno = 0;
		frm_tab[count].fr_dirty = 0;
		count++;
	}

	restore(ps);
	return OK;

}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  	STATWORD ps;
	disable(ps);
	int count = 0;
	*avail = -1;
	int frame_number;
	for(count = 0; count < NFRAMES; count++) {
		if(frm_tab[count].fr_status == FRM_UNMAPPED) {
			*avail = count;
			restore(ps);
			return OK;
		}
	}

	if(page_replace_policy == SC) {
		frame_number = getFrame_SC();

		if(frame_number == -1){
			restore(ps);
			return SYSERR;
		}

		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	} else if(page_replace_policy == AGING) {
		frame_number = getFrame_Aging();

		if(frame_number == -1) {
			restore(ps);
			return SYSERR;
		}

		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	}

	restore(ps);
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	STATWORD ps;
	disable(ps);

	unsigned long virtual_address;
	unsigned long pd_base_register;	
	int pt_frame;
	int bs_reference, frame_pid, page_number;

	unsigned int pt_offset;
	unsigned int pd_offset;
	pd_t *pd_entry; 
	pt_t *pt_entry;

	if(frm_tab[i].fr_type == FR_PAGE){
		frame_pid = frm_tab[i].fr_pid;
		pd_base_register = proctab[frame_pid].pdbr;
		virtual_address = frm_tab[i].fr_vpno;

		pd_offset = virtual_address / 1024;		
		pd_entry = pd_base_register + (pd_offset * sizeof(pd_t));
		pt_offset = virtual_address & 1023;
		pt_entry = (pd_entry->pd_base*NBPG) + (pt_offset * sizeof(pt_t));

		page_number = frm_tab[i].fr_vpno - proctab[frame_pid].vhpno;
		bs_reference = proctab[frm_tab[i].fr_pid].store;
		write_bs((i + FRAME0)*NBPG, bs_reference, page_number);

		pt_entry->pt_pres = 0;
		pt_frame = pd_entry->pd_base - FRAME0;
		frm_tab[pt_frame].fr_refcnt -= 1;

		if(frm_tab[pt_frame].fr_refcnt == 0){
			pd_entry->pd_pres = 0;
			frm_tab[pt_frame].fr_type = FR_PAGE;
			frm_tab[pt_frame].fr_status = FRM_UNMAPPED;
			frm_tab[pt_frame].fr_pid = -1;
			frm_tab[pt_frame].fr_vpno = BS_VIRTUAL_BASE_PAGE;
		}

		restore(ps);
		return OK;
	}

	restore(ps);
	return SYSERR;
}

void set_pageDirectory(int pid) {
	int frame_number = 0;
	int count = 0;	
	pd_t *pagr_dir_entry;

	if(get_frm(&frame_number) != SYSERR){
		proctab[pid].pdbr = (FRAME0 + frame_number)*NBPG;
		frm_tab[frame_number].fr_pid = pid;
		frm_tab[frame_number].fr_status = FRM_MAPPED;
		frm_tab[frame_number].fr_type = FR_DIR;
		pagr_dir_entry = (pd_t*)((FRAME0+ frame_number)*NBPG);

		while(count < NFRAMES) {
			pagr_dir_entry[count].pd_write = 1;	
			if(count < 4) {
				pagr_dir_entry[count].pd_base = FRAME0 + count;
				pagr_dir_entry[count].pd_pres = 1;
				// pagr_dir_entry[count].pd_user = 0;
			}else {
				pagr_dir_entry[count].pd_pres = 0;
				// pagr_dir_entry[count].pd_user = 0;
			}
			count++;
		}
	} else {
		kprintf("set_pageDirectory: Error!");
	}
}

void page_directory_allocation(int pid)
{
	int frame_no = 0;	
	pd_t *pdir_entry;

	get_frm(&frame_no);

	proctab[pid].pdbr = (frame_no + FRAME0) * NBPG;
	frm_tab[frame_no].fr_pid = pid;
	frm_tab[frame_no].fr_type = FR_DIR;
	frm_tab[frame_no].fr_status = FRM_MAPPED;
	
	pdir_entry = (FRAME0+ frame_no) * NBPG;
	int count;
	for(count = 0; count < 1024; count++){
		pdir_entry[count].pd_write = 1;

        if(count < 4) {
            pdir_entry[count].pd_pres = 1;
            pdir_entry[count].pd_base = FRAME0 + count;
        } else {
            pdir_entry[count].pd_pres = 0;
        }
	}
}

void init_frame_fifo(void)
{
	int i = 0;
	for(i = 0; i < NFRAMES; i++) {
		fifo_frame[i].frm_id = i;
		fifo_frame[i].next_frame = -1;
	}
}

void evict_frame(int pid)
{
	int count = 0;

	while(count < NFRAMES) {
		if(frm_tab[count].fr_pid == pid) {
			frm_tab[count].fr_status = FRM_UNMAPPED;
		  	frm_tab[count].fr_pid = -1;
		  	frm_tab[count].fr_type = FR_PAGE;
		  	frm_tab[count].fr_vpno = BS_VIRTUAL_BASE_PAGE;
		  	frm_tab[count].fr_dirty = 0;
		  	frm_tab[count].fr_refcnt = 0;

		}
		count++;
	}
}

int set_page_table()
{
	int count = 0;
	int frame_number;
	unsigned int frame_address;
	pt_t *page_table_pointer;

	if(get_frm(&frame_number)!=SYSERR) {

		frame_address = (FRAME0 + frame_number)*NBPG;
		page_table_pointer = (pt_t*)frame_address;

        while(count < NFRAMES) {
			page_table_pointer[count].pt_write = 1;
    	    page_table_pointer[count].pt_global = 0;
        	page_table_pointer[count].pt_acc = 0;
    	    page_table_pointer[count].pt_mbz = 0;
        	page_table_pointer[count].pt_pcd = 0;
	        page_table_pointer[count].pt_avail = 0;
    	    page_table_pointer[count].pt_base = 0;
        	page_table_pointer[count].pt_pres = 0;
	        page_table_pointer[count].pt_pwt = 0;
    	    page_table_pointer[count].pt_dirty = 0;
        	page_table_pointer[count].pt_user = 0;

			page_table_pointer = (pt_t*)frame_address;
			count++;
		}
		return frame_number;
	}
	return SYSERR;	
}

int getFrame_SC() {
	unsigned long virtualAddress;
	unsigned long pdbr;
	unsigned int pt_offset;
	unsigned int pd_offset;
	
	pd_t *pd_entry; 
	pt_t *pt_entry;
	int frame_pid;
	int stopFlag = 0;
	int iterator_ft = 0;

	while(!stopFlag) {
		if(frm_tab[iterator_ft].fr_type==FR_PAGE){
			virtualAddress = frm_tab[iterator_ft].fr_vpno;

			/* Calculating page directory offset and page table offset */
			pd_offset = virtualAddress/1024;
			pt_offset = virtualAddress&1023;

			frame_pid = frm_tab[iterator_ft].fr_pid;
			pdbr = proctab[frame_pid].pdbr;

			pd_entry = pdbr + (pd_offset*sizeof(pd_t));
			pt_entry = (pd_entry->pd_base*NBPG) + (pt_offset*sizeof(pt_t));

			if(pt_entry->pt_acc == 0){
				stopFlag=1;
				return iterator_ft;
			} else{
				pt_entry->pt_acc=0;
			}
		}
		iterator_ft++;
		iterator_ft = iterator_ft % NFRAMES;
	}

	/* Should not reach here */
	return -1;
}

int getFrame_Aging() {
	unsigned long virtualAddress;
	unsigned long pdbr;
	unsigned int pt_offset;
	unsigned int pd_offset;

	pd_t *pd_entry; 
	pt_t *pt_entry;
	int frame_pid;

	int iterator_ft = 0;
	int minIndex = -1;
	int minAge = 300;
	int tempVal;

	while(iterator_ft<NFRAMES)
	{
		if(frm_tab[iterator_ft].fr_type==FR_PAGE){
			virtualAddress = frm_tab[iterator_ft].fr_vpno;

			/* Calculating page directory offset and page table offset */
			pd_offset = virtualAddress/1024;
			pt_offset = virtualAddress&1023;

			frame_pid = frm_tab[iterator_ft].fr_pid;
			pdbr = proctab[frame_pid].pdbr;

			pd_entry = pdbr+(pd_offset*sizeof(pd_t));
			pt_entry = (pd_entry->pd_base*NBPG)+(pt_offset*sizeof(pt_t));

			frm_tab[iterator_ft].fr_age>>=1;
			if(pt_entry->pt_acc == 1){
				tempVal = frm_tab[iterator_ft].fr_age+128;
				frm_tab[iterator_ft].fr_age = 255 < tempVal ? 255 : tempVal;	
			}		
			if(frm_tab[iterator_ft].fr_age < minAge){
				minAge=frm_tab[iterator_ft].fr_age;
				minIndex=iterator_ft;
			}
		}
		iterator_ft++;
	}
	return minIndex;
}

void clear_frame(int i){
  frm_tab[i].fr_status = FRM_UNMAPPED;
  frm_tab[i].fr_pid = -1;
  frm_tab[i].fr_type = FR_PAGE;
  frm_tab[i].fr_vpno = 0;
  frm_tab[i].fr_dirty = 0;
  frm_tab[i].fr_refcnt = 0;
  frm_tab[i].fr_age = 0;
}

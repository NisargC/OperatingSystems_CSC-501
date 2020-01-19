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
	//*avail = -1;
	int frame_number;
	for(count = 0; count < NFRAMES; count++) {
		if(frm_tab[count].fr_status == FRM_UNMAPPED) {
			*avail = count;
			restore(ps);
			return OK;
		}
	}

	if(page_replace_policy == SC) {
		frame_number = frame_removal_SC_policy();

		if(frame_number == -1){
			restore(ps);
			kprintf("Syserr in get_frm SC");
			return SYSERR;
		}

		kprintf("Frame to be freed is %d",frame_number);
		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	} else if(page_replace_policy == AGING) {
		frame_number = frame_removal_AGING_policy();

		if(frame_number == -1) {
			restore(ps);
			kprintf("Syserr in get_frm Aging");
			return SYSERR;
		}
		free_frm(frame_number);
		*avail = frame_number;
		restore(ps);
		return OK;
	}

	restore(ps);
	kprintf("Syserr in get_frm final");
	return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	unsigned long v_addr,pdbr;	
	unsigned int pt_offset,pd_offset;
	
	pd_t *pd_entry; 
	pt_t *pt_entry;
	int backing_store,page_num,frame_tab_pid;

	switch(frm_tab[i].fr_type){

		case FR_PAGE:
			
			frame_tab_pid = frm_tab[i].fr_pid;
			v_addr = frm_tab[i].fr_vpno;
			
			pdbr = proctab[frame_tab_pid].pdbr;

			pd_offset = v_addr / NFRAMES;
			pt_offset = v_addr&1023;
			
			pd_entry = pdbr + (pd_offset*sizeof(pd_t));
			pt_entry = (pd_entry->pd_base*NBPG) + (pt_offset*sizeof(pt_t));
			backing_store = proctab[frm_tab[i].fr_pid].store;
			page_num = frm_tab[i].fr_vpno-proctab[frame_tab_pid].vhpno;
			
			if(i < FR_PAGE){
				kprintf("ERROR while freeing a frame: Invalid frame number passed.\n");
				restore(ps);
				return SYSERR;
			}

			write_bs((i+FRAME0)*NBPG, backing_store, page_num);
			pt_entry->pt_pres = 0;
			frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt -= 1;
			int temp = frm_tab[pd_entry->pd_base-FRAME0].fr_refcnt;
			if(temp == 0){
				frm_tab[pd_entry->pd_base-FRAME0].fr_pid = -1;
				frm_tab[pd_entry->pd_base-FRAME0].fr_status = FRM_UNMAPPED;
				frm_tab[pd_entry->pd_base-FRAME0].fr_type = FR_PAGE;
				frm_tab[pd_entry->pd_base-FRAME0].fr_vpno = VPN_BASE;
				pd_entry->pd_pres = 0;
			}

			break;
		case FR_TBL:
			if(i < FR_TBL){
				kprintf("ERROR while freeing a frame.\n");
				restore(ps);
				return SYSERR;
			}

			break;
		case FR_DIR:
			if(i < FR_DIR){
				kprintf("ERROR while freeing a frame.\n");
				restore(ps);
				return SYSERR;
			}
			break;

		default:
			break;
	}

 	restore(ps);
	return OK;
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
			page_table_pointer[count].pt_write = 0;
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
	kprintf("Syserr in set_page_table");
	return SYSERR;

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

void initialize_frames_SC_AGING()
{
	int i = 0;
	while(i < NFRAMES){
		fifo_frame[i].frm_id = i;
		fifo_frame[i].next_frame = -1;
		fifo_frame[i].frm_age = 0;
		i++;
	}
}

void insert_frame_SC_AGING(int frame_num)
{
	STATWORD ps;
	disable(ps);
	int nxt_frm = -1;
	int curr_frm = -1;
	
	//when queue is empty
	if(fifo_head == -1){
		fifo_head = frame_num;
		restore(ps);
		return OK;
	}
	//if queue has some elements already
	else{
		nxt_frm = fifo_frame[fifo_head].next_frame;
		curr_frm = fifo_head;
	}
	//finding the last frame/element in the queue
	while(nxt_frm != -1){
		curr_frm = nxt_frm;
		nxt_frm = fifo_frame[nxt_frm].next_frame;
	}
	
	fifo_frame[curr_frm].next_frame = frame_num;
	fifo_frame[frame_num].next_frame = -1;
	
	restore(ps);
	return OK;
}

int frame_removal_SC_policy()
{
	STATWORD ps;
	disable(ps);

	int frame_number=0,curr_frm=0,next_frame = -1,prev_frame = -1,page_table_new,page_offset;
	unsigned int virt_pt_offset,virt_pd_offset;
	unsigned long pdbr,virtual_address; 
	
	virt_addr_t *virt_addr; 
	
	pd_t *pd_entry;
	pt_t *pt_entry; 

	while(curr_frm != -1){

		virtual_address = frm_tab[curr_frm].fr_vpno;

		virt_addr = (virt_addr_t*)&virtual_address;
		virt_pt_offset = virt_addr->pt_offset;
		virt_pd_offset = virt_addr->pd_offset;

		pdbr = proctab[currpid].pdbr;
		pd_entry = pdbr + virt_pd_offset * sizeof(pd_t);
		pt_entry = (pt_t*)(pd_entry->pd_base*NBPG + virt_pt_offset*sizeof(pt_t));
		frame_number = fifo_head;
		
		if(pt_entry->pt_acc == 0){
			//when queue head has pt_acc = 0 and hence should be removed
			if(prev_frame==-1){
				fifo_head = fifo_frame[curr_frm].next_frame;
				fifo_frame[curr_frm].next_frame = -1;
				restore(ps);
				return (frame_number);
			}
			//some node after head has to be removed
			else{
				fifo_frame[prev_frame].next_frame = fifo_frame[curr_frm].next_frame;
				fifo_frame[curr_frm].next_frame = -1;
				restore(ps);
				return(frame_number);
			}

		}
		else{
			//setting access bit to 0 if previously set to 1
			pt_entry->pt_acc = 0;
		}

		kprintf("FIFO Head %d",fifo_head);

		prev_frame = curr_frm;
		curr_frm = fifo_frame[curr_frm].next_frame;		
	}

	//if the head of queue is to be removed
	fifo_head = fifo_frame[curr_frm].next_frame;
	fifo_frame[curr_frm].next_frame = -1;

	restore(ps);
	return (frame_number);

}

int frame_removal_AGING_policy(){
	STATWORD ps;
	disable(ps);
	
	int frame_number=0,curr_frm,next_frame = -1,prev_frame = -1,page_table_new,page_offset;
	unsigned int virt_pt_offset,virt_pd_offset;
	unsigned long pdbr,virtual_address; 
	
	virt_addr_t *virt_addr; 
	
	pd_t *pd_entry;
	pt_t *pt_entry; 

	while(curr_frm != -1){

		virtual_address = frm_tab[curr_frm].fr_vpno;

		virt_addr = (virt_addr_t*)&virtual_address;
		virt_pt_offset = virt_addr->pt_offset;
		virt_pd_offset = virt_addr->pd_offset;

		pdbr = proctab[currpid].pdbr;
		pd_entry = pdbr + virt_pd_offset * sizeof(pd_t);
		pt_entry = (pt_t*)(pd_entry->pd_base*NBPG + virt_pt_offset*sizeof(pt_t));
		frame_number = fifo_head;
		
		//decrementing the age of each element by half
		fifo_frame[curr_frm].frm_age = fifo_frame[curr_frm].frm_age/2;

		if(pt_entry->pt_acc == 1){
			//incrementing age of element and comparing with 255
			int temp = fifo_frame[curr_frm].frm_age + 128;
			if(temp < 255)
				fifo_frame[curr_frm].frm_age = temp;
			else
				fifo_frame[curr_frm].frm_age = 255;
		}
		//finding the frame with minimum age value
		if(fifo_frame[curr_frm].frm_age < fifo_frame[frame_number].frm_age)
			frame_number = curr_frm;

		prev_frame = curr_frm;
		curr_frm = fifo_frame[curr_frm].next_frame;		
	}

	restore(ps);
	return (frame_number);
}

/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
    STATWORD ps;
	disable(ps);

    /* Doing error checking for input values as xmmap is a SYSCALL */
    if(npages > 256 || npages < 1){
        kprintf("XMMAP: No of pages requested is out of bounds");
        return SYSERR;
    }

    if(source < 0 || source > (MAX_BS - 1)) {
        kprintf("XMMAP: Backing store ID out of bounds");
        restore(ps);
        return SYSERR;
    }

    if(virtpage < VPN_BASE){
        kprintf("XMMAP: Virtual page address is out of bounds");
        return SYSERR;
    }

    /* All inputs are values as expected */
    bsm_map(currpid, virtpage, source, npages);
    restore(ps);
    return OK;

}


/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
    STATWORD ps;
	disable(ps);

    /* Doing error checking for input values as xmunmap is a SYSCALL */
    if(virtpage < VPN_BASE){
        kprintf("XMUNMAP: Virtual page address is out of bounds");
        restore(ps);
        return SYSERR;
    }

    bsm_unmap(currpid, virtpage);
    restore(ps);
    return OK;
}

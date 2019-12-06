#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */
int ldelete(int lockdescriptor)
{
    STATWORD ps;
    disable(ps);

    struct lockentry *lptr;
    int lock = lockdescriptor / 10;
    int pid;
    int lock_index = lockdescriptor - lock * 10;

    /* Should not allow deletion of a free lock, or the lock value is an error */
    if (lock < 0 || lock > NLOCKS || locks[lock].lockState == LFREE) {
        restore(ps);
        return (SYSERR);
    }

    lptr = &locks[lock];
    lptr->lockState = LFREE;

	/* Should strictly be 0 or 1 */
    if (lock_index - locks_traverse >= 2) {
        restore(ps);
        return (SYSERR);
    }

    /* Checking if queue empty or not */
    if (nonempty(lptr->lockqueueHead)) {
        pid = getfirst(lptr->lockqueueHead);
        while (pid != EMPTY) {
            /* Set the process lock to DELETED */
            proctab[pid].plockret = DELETED;
            ready(pid, RESCHNO);
        }
        resched();
    }
    restore(ps);
    return (OK);
}

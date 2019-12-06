#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include "lock.h"

/* Release all locks */
int releaseall(int numlocks, int args, ...)
{
    STATWORD ps;
    disable(ps);

    int queue_node, lock_prio, lock, lock_index, reader_lock, writer_lock, type_lock;
    unsigned long readlock_time, writelock_time;
    struct lockentry *lptr;
    int lockCount;
    for (lockCount = 0; lockCount < numlocks; lockCount++) {
        int releaseProcFlag = -1;
        
        /* Getting the lock based on how many were passed and which lock count we are on */
        lock = (int)(*((&args) + lockCount) / 10);
        lock_index = *((&args) + lockCount) - lock * 10;

        lptr = &locks[lock];

        /* Should not come here with lock not acquired for current proc. Set to 1 */
        if (lptr->procLog[currpid] == 0) {
            lptr->procLog[currpid] = lptr->procLog[currpid] + 1;
            /*swapPriority(lptr->procLog[currpid], lptr->procLog[currpid] + 1);*/
        }

        lptr->procLog[currpid] = 0;
        proctab[currpid].lockLog[lock] = 0;
        updateLockPriority(currpid);

        /* Invalid check for lockID */
        if (lock < 0 || lock > NLOCKS) {
            restore(ps);
            return (SYSERR);
        }

        if (lptr->num_writer > 0) {
            lptr->num_writer -= 1;
        } else {
            if (lptr->num_reader > 0) {
                lptr->num_reader -= 1;
            }
        }

        writer_lock = -1;
        reader_lock = -1;
        readlock_time = 0;
        writelock_time = 0;

        queue_node = q[lptr->lockqueueTail].qprev;
        lock_prio = type_lock = 0;

        if (q[queue_node].qkey == q[q[queue_node].qprev].qkey) {
            lock_prio = q[queue_node].qkey;

            while (q[queue_node].qkey == lock_prio) {
                if (q[queue_node].qtype == READ) {
                    if (q[queue_node].qtime > readlock_time)
                        reader_lock = queue_node;
                }
                else if (q[queue_node].qtype == WRITE) {
                    if (q[queue_node].qtime > writelock_time)
                        writer_lock = queue_node;
                }

                if (reader_lock >= 0 && writer_lock >= 0) {
                    if (readlock_time - writelock_time < 1000 || writelock_time - readlock_time < 1000) {
                        type_lock = writer_lock;
                    }
                    else if (readlock_time > writelock_time) {
                        type_lock = reader_lock;
                    }
                    else if (readlock_time < writelock_time) {
                        type_lock = writer_lock;
                    }
                }
                queue_node = q[queue_node].qprev;
            }

            if (lock < 0) {
                while (lptr->lockState == PRFREE) {
                    lptr->lockState = READ;
                    lptr->num_reader = lptr->procLog[lock];
                    lptr->lockqueueHead = q[type_lock].qkey;
                }
            }
            if (releaseProcFlag == -1 && lptr->num_reader == 0) {
                if (q[type_lock].qtype == WRITE && lptr->num_writer == 0) {
                    releaseWriter(lock, type_lock);
                    releaseProcFlag = 0;
                }
            }
            if (lptr->num_writer == 0 && releaseProcFlag == -1 && q[type_lock].qtype == WRITE) {
                checkReader(lock);
                releaseProcFlag = 0;
            }
        }
        if (q[queue_node].qkey != q[q[queue_node].qprev].qkey) {
            if (q[queue_node].qtype == READ) {
                if (lptr->num_writer == 0)
                    checkReader(lock);
            }
            else if (q[queue_node].qtype == WRITE) {
                if (lptr->num_reader == 0 && lptr->num_writer == 0)
                    releaseWriter(lock, queue_node);
            }
            continue;
        }
    }

    restore(ps);
    resched();
    return (OK);
}

void checkReader(int lockid) {
    struct lockentry *lptr;
    lptr = &locks[lockid];
    int x, q_prev, max = -1;

    /* Getting max from all process priorities in the queue */
    for (x = q[lptr->lockqueueTail].qprev; x != lptr->lockqueueHead; x = q[x].qprev) {
        if (q[x].qkey > max && q[x].qtype == WRITE) {
            max = q[x].qkey;
        }
    }
    for (x = q[lptr->lockqueueTail].qprev; x != lptr->lockqueueHead;) {
        if (q[x].qkey >= max && q[x].qtype == READ) {
            q_prev = q[x].qprev;
            releaseWriter(lockid, x);
            x = q_prev;
        }
    }
}

void releaseWriter(int lockID, int p) {
    struct lockentry *lptr;
    lptr = &locks[lockID];
    lptr->procLog[p] = 1;
    proctab[currpid].lockLog[lockID] = 1;
    if (q[p].qtype == READ) {
        lptr->num_reader = lptr->num_reader + 1;
    }
    else if (q[p].qtype == WRITE) {
        lptr->num_writer = lptr->num_writer + 1;
    }

    modifyLockPriority(lockID);
    int procID;
    for (procID = 0; procID < NPROC; procID++) {
        if (lptr->procLog[procID] > 0)
            updateLockPriority(procID);
    }

    /* Dequeue the process as we have updated the priorities */
    dequeue(p);
    ready(p, RESCHNO);
}

void updateLockPriority(int procID) {
    struct pentry *pptr;
    pptr = &proctab[procID];
    int lockID = 0, maxPrio = -1;

    /* Get the max priority for all processes from the lock log for each process */
    while (lockID < NLOCKS) {
        if (pptr->lockLog[lockID] > 0) {
            if (locks[lockID].lockPriority > maxPrio)
                maxPrio = locks[lockID].lockPriority;
        }
        lockID++;
    }

    /* Ramping up the inherited priority for the process if it has a priority lesser than max */
    if (pptr->pprio <= maxPrio) {
        pptr->pinh = maxPrio;
    } else if (pptr->pprio > maxPrio) {
        pptr->pinh = 0;
    }
}

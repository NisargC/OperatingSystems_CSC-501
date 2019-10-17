#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "sched.h"
#include "math.h"
#include "lab1.h"

unsigned long currSP; /* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:       Upon entry, currpid gives current process id.
 * Proctab[currpid].pstate gives correct NEXT state for
 * current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
    register struct pentry *old_ptr;
    register struct pentry *new_ptr;

    int random;
    int next_proc;
    if (scheduler_class == EXPDISTSCHED) {
        double lambda = 0.1; /* Distribution scheduler random value */
        random = (int)expdev(lambda);
        next_proc = nextproc_picker_edt(random);
        old_ptr = &proctab[currpid];
        if (old_ptr->pstate == PRCURR && ((next_proc == NULLPROC) || (old_ptr->pprio > random && old_ptr->pprio < q[next_proc].qkey))) {
#ifdef RTCLOCK
            preempt = QUANTUM; /* reset preemption counter */
#endif
            return (OK);
        }
    }
    else if (scheduler_class == LINUXSCHED) {
        int epoch = 0;
        old_ptr = &proctab[currpid];
        old_ptr->goodness = old_ptr->goodness - old_ptr->counter + preempt;
        old_ptr->counter = preempt;

        if (old_ptr->counter <= 0 || currpid == NULLPROC) {
            /* Process has no more clicks left and it has used up it's quantum time
             * Set goodness and counter to 0
             */
            old_ptr->counter = 0;
            old_ptr->goodness = 0;
        }

        int max_goodness = 0;
        int i = 0;
        int new_process = 0;

        /* Calculating max_goodness from all available processes in proctab */
        for (i = q[rdytail].qprev; i != rdyhead; i = q[i].qprev) {
            if (proctab[i].goodness > max_goodness) {
                new_process = i;
                max_goodness = proctab[i].goodness;
            }
        }

        if ((old_ptr->pstate != PRCURR || old_ptr->counter == 0) && max_goodness == 0) {
            /* Start new epoch and update the goodness values based on counters */
	    if(epoch == 0){
                epoch = 1;
                int count;
                struct pentry *p;
                for (count = 0; count < NPROC; count++)
                {
                    p = &proctab[count];
                    if (p->pstate != PRFREE) {
                        if (p->counter == p->time_quant || p->counter == 0) {
                            p->time_quant = p->pprio;
                        }
                        else {
                            p->time_quant = p->pprio + ((p->counter) / 2);
                        }
                        p->counter = p->time_quant;
                        p->goodness = p->pprio + p->time_quant;
                    }
                }
                preempt = old_ptr->counter;
                old_ptr = &proctab[currpid];
	    }
            if (max_goodness == 0) {
                /* No process to schedule, schedule NULLPROC if not already scheduled */
                if (currpid == NULLPROC) {
                    return OK;
                }
                else {
                    if (old_ptr->pstate == PRCURR) {
                        old_ptr->pstate = PRREADY;
                        insert(currpid, rdyhead, old_ptr->pprio);
                    }
                    new_process = NULLPROC;
                    new_ptr = &proctab[new_process];
                    new_ptr->pstate = PRCURR;
                    dequeue(new_process);
                    currpid = new_process;
#ifdef RTCLOCK
                        preempt = QUANTUM;
#endif
                    /* Switch context from old process to new process */
                    ctxsw((int)&old_ptr->pesp, (int)old_ptr->pirmask, (int)&new_ptr->pesp, (int)new_ptr->pirmask);
                    return OK;
                }
            }
        }
        else if (old_ptr->pstate == PRCURR && old_ptr->goodness > 0 && old_ptr->goodness >= max_goodness) {
	    /* If current process has goodness higher than all other processes, then keep that process running
 	     * So no need for context switch. Just reset preempt and continue running
 	     * */
            preempt = old_ptr->counter;
            return (OK);
        }
        else if (max_goodness > 0 && (old_ptr->pstate != PRCURR || old_ptr->goodness < max_goodness || old_ptr->counter == 0)) {
            if (old_ptr->pstate == PRCURR) {
                /* Change process to Ready state and insert it into ready queue */
                old_ptr->pstate = PRREADY;
                insert(currpid, rdyhead, old_ptr->pprio);
            }
            /* Dequeue the new process from ready queue and do normal context switch */

            new_ptr = &proctab[new_process];
            new_ptr->pstate = PRCURR;
            dequeue(new_process);
            preempt = new_ptr->counter;
            currpid = new_process;
            /* Switch context from old process to new process */
            ctxsw((int)&old_ptr->pesp, (int)old_ptr->pirmask, (int)&new_ptr->pesp, (int)new_ptr->pirmask);
            return OK;
        }
        else {
            /* Raising system error as this case will only be achieved due to erroneus conditions */
            return SYSERR;
        }
    }
    else {
        /* no switch needed if current process priority higher than next */
        if (((old_ptr = &proctab[currpid])->pstate == PRCURR) &&
            (lastkey(rdytail) < old_ptr->pprio)) {
            return (OK);
        }
    }
    /* force context switch */

    if (old_ptr->pstate == PRCURR) {
        old_ptr->pstate = PRREADY;
        insert(currpid, rdyhead, old_ptr->pprio);
    }

    /* remove highest priority process at end of ready list */
    if (scheduler_class == EXPDISTSCHED) {
        new_ptr = &proctab[(currpid = lastproc_picker_edt(next_proc))];
    }
    else {
        new_ptr = &proctab[(currpid = getlast(rdytail))];
    }
    new_ptr->pstate = PRCURR; /* mark it currently running    */
#ifdef RTCLOCK
    preempt = QUANTUM; /* reset preemption counter     */
#endif

    ctxsw((int)&old_ptr->pesp, (int)old_ptr->pirmask, (int)&new_ptr->pesp, (int)new_ptr->pirmask);

    /* The OLD process returns here when resumed. */
    return OK;
}


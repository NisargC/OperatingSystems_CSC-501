#ifndef _LOCK_H_
#define _LOCK_H_

#define NLOCKS 50       /* Number of allowed locks */

#define READ   0        /* Whether it is a read lock or a write lock */
#define WRITE  1

#define LFREE  0        /* Whether the lock is free or used */
#define LUSED  1

struct lockentry {
    char lockState;         /* The state LFREE or LUSED	*/
    int  num_reader;        /* Number of readers contending for the lock */
    int  num_writer;        /* Number of writers contending for the lock */
    int  lockqueueHead;     /* Queue index of head of list */
    int  lockqueueTail;     /* Queue index of tail of list */
    int  lockPriority;      /* Lock priority */
    int  procLog[NPROC];    /* Process that acquires this lock */
};

extern struct lockentry locks[];
extern int nextlock;
extern int locks_traverse;
extern unsigned long ctr1000;

extern void modifyLockPriority(int lid);
extern void updateLockPriority(int pid);
extern void swapPriority(int,int);

#endif

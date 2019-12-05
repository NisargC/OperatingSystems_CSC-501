#ifndef _LOCK_H_
#define _LOCK_H_

#define NLOCKS 50       /* Number of allowed locks */

#define READ   0        /* Whether it is a read lock or a write lock */
#define WRITE  1

#define LFREE  0        /* Whether the lock is free or used */
#define LUSED  1

struct lockentry {
    char lockState;
    int  num_reader;
    int  num_writer;
    int  lockqueueHead;
    int  lockqueueTail;
    int  lockPriority;
    int  procLog[NPROC];
};

extern struct lockentry locks[];
extern int nextlock;
extern int locks_traverse;
extern unsigned long ctr1000;

extern void modifyLockPriority(int lid);
extern void updateLockPriority(int pid);
extern void getInheritedPrio(int lid);
extern void releaseWriter(int,int);
extern void swapPriority(int,int);

#endif

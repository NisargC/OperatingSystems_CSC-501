#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate()
{
	STATWORD ps;    
	disable(ps);

	int status=-1,lockIndex,lockCounter=0;

	/* Go through all the available locks to see if any are free */
	while(lockCounter < NLOCKS && status == -1){
		lockIndex = nextlock--;
		if(nextlock < 0){
			locks_traverse += 1;
			nextlock = NLOCKS - 1;
		}
		if(locks[lockIndex].lockState != LFREE){
			status = -1;
		} else if(locks[lockIndex].lockState == LFREE){
			status = lockIndex*10 + locks_traverse;
			locks[lockIndex].lockState = LUSED;
			locks[lockIndex].num_reader = 0;
			locks[lockIndex].num_writer = 0;
			break;
		}
		lockCounter++;
	}

	/* Status did not get updated as all the locks are being used, returning SYSERR */
	if(status == -1){
		restore(ps);
		return(SYSERR);
	}

	restore(ps);
	return(status);
}

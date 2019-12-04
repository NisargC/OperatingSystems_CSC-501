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

	int status=-1,index,counter=0;

	/* Go through all the available locks to see if any are free */
	while(counter < NLOCKS && status == -1){
		index = nextlock--;
		if(nextlock < 0){
			locks_traverse += 1;
			nextlock = NLOCKS - 1;
		}
		if(locks[index].lockState != LFREE){
			status = SYSERR;
		} else if(locks[index].lockState == LFREE){
			status = index*10 + locks_traverse;
			locks[index].lockState = LUSED;
			locks[index].num_reader = 0;
			locks[index].num_writer = 0;
			break;
		}
		counter++;
	}

	/* Got SYSERR from the above loop as all the locks are being used */
	if(status==-1){
		restore(ps);
		return(SYSERR);
	}

	restore(ps);
	return(status);
}

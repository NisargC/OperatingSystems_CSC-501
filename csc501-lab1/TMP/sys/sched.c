#include<kernel.h>
#include <q.h>
#include <proc.h>
#include "sched.h"

int getschedclass() {  
	return(scheduler_class);
}

void setschedclass (int sched_class) {
	scheduler_class = sched_class;
}

int nextproc_picker_edt(int random) {
	int currproc;
	int prevproc;
	currproc = q[rdytail].qprev;
	prevproc = q[currproc].qprev;

	while(prevproc < NPROC && random < q[prevproc].qkey) {
		if(q[currproc].qkey != q[prevproc].qkey) {
			// Only when priorities are different will we move the current process
			currproc = prevproc;
		}
		prevproc = q[prevproc].qprev;
	}
	
	if(currproc >= NPROC){
		return NULLPROC;
	} else {
		return currproc;
	}
}

int lastproc_picker_edt(int proc){
	if(proc > NPROC){
		return EMPTY;
	} else {
		return dequeue(proc);
	}
}

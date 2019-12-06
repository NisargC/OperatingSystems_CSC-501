#include <kernel.h>
#include <proc.h>
#include <stdio.h>

unsigned long *esp;

void printprocstks(int priority){
	struct pentry *proc;
	int i;

	printf("\nvoid printprocstks(int priority)\n");
	for(i = 0; i< NPROC; i++ ) {
	
	// Getting the processes from the proctab table
	proc = &proctab[i];
		if(proc -> pstate != PRFREE) {
			if(proc -> pprio > priority) {

				printf("Process [%s]\n", proc -> pname);
				printf("\tpid: %d\n",i);
				printf("\tpriority: %d\n",proc -> pprio);	
				printf("\tbase: 0x%08x\n",proc -> pbase);
				printf("\tlimit: 0x%08x\n",proc -> plimit);
				printf("\tlen: %d\n",proc -> pstklen);

				if(proc -> pstate == PRCURR) {
					asm("movl %esp, esp");
				}
				else {
					esp = (unsigned long *)proc -> pesp;
				}
				printf("\tpointer : 0x%08x\n",esp);
			}
		}	
	}
}

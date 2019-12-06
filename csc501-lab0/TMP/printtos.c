#include<stdio.h>

unsigned long *ebp;
unsigned long *esp;

void printtos(){
	printf("\nvoid printtos()\n");
	asm("mov %ebp, ebp");
	asm("mov %esp, esp");

	printf("Before[0x%08x]: 0x%08x\n", ebp + 2, *(ebp + 2));
	printf("After[0x%08x]: 0x%08x\n", ebp, *ebp);
	
	int counter = ebp - esp + 1;
	int i = 1;
	while(i < 15 && counter > 0){
		printf("\telement[0x%08x] 0x%08x\n",esp + i, *(esp + i) );
		i++;
		counter++;
	}
	return;
}

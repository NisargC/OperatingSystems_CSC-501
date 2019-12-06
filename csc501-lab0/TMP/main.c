/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include"printsegaddress.c"
#include"printtos.c"
#include"printprocstks.c"
#include"printsyscallsummary.c" 
/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

extern long zfunction(long);
extern void printsegaddress();
extern void printprocstks(int);
extern void printtos();
extern void printsyscallsummary();
extern void syscallsummary_start();
extern void syscallsummary_stop();

int prX;
void halt();
prch(c)
char c;
{
	int i;
	sleep(1);	
}

int main()
{
	int param = 0xaabbccdd;
	int priority = 0;
	printf("\nNisarg Chokshi (nmchoks2) - \n");

	printf("\nlong zfunction(long param)\n");
	printf("zfunction(0x%x) = 0x%x\n", param, zfunction(param));
	printsegaddress();
	printtos();
	printprocstks(priority);
	syscallsummary_start();
	resume(prX = create(prch,2000,20,"proc X",25,'A'));
	sleep(10);	
	syscallsummary_stop();
	printsyscallsummary();
	kprintf("\n\nHello World, Xinu lives\n\n");
	return 0;
}


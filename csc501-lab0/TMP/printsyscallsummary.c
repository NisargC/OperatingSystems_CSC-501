#include<stdio.h>
#include<kernel.h>
#include<proc.h>

extern int start_stop_flag;
char proc_name[27][NPROC]={"freemem","chprio","getpid","getprio","gettime","kill","receive","recvclr","recvtim","resume","scount","sdelete","send","setdev","setnok","screate","signal","signaln","sleep","sleep10","sleep100","sleep1000","sreset","stacktrace","suspend","unsleep","wait"};

void syscallsummary_start(){
	start_stop_flag=1;
	int i=0;
	int j=0;
	for(i=0; i<NPROC; i++){
	struct pentry *proc = &proctab[i];
		for(j=0; j<27; j++){
			proc->call_count_array[j]=0;
			proc->start_time_array[j]=0;
			proc->end_time_array[j]=0;
		}
	}
}

void syscallsummary_stop(){
	start_stop_flag=0;
}

void printsyscallsummary(){
	printf("\nvoid printsyscallsummary()\n");
	int i = 0, j = 0;
	for(j=1; j<NPROC; j++){
		struct pentry *proc = &proctab[j];
		if(strcmp(proc->pname,"") != 0)
			printf("Process [pid:%d]\n",j);
		for(i=0; i<27; i++){
			if(proc->call_count_array[i]>0){
				 long exec_time = 0;
				 if(proc->start_time_array[i] <= proc->end_time_array[i]){
					exec_time = (proc->end_time_array[i] - proc->start_time_array[i])/proc->call_count_array[i];}
				 printf("\tSyscall: %s, count: %d, average execution time: %d (ms)\n",proc_name[i],proc->call_count_array[i],exec_time);
			}
       		 }		
	
	}
}

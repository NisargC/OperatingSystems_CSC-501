#ifndef _sched_h_
#define _sched_h_

int scheduler_class;

int getschedclass();
void setschedclass (int);
int nextproc_picker_edt(int);
int lastproc_picker_edt(int);

#endif

## CSC 501 PA2
 Implemented two new scheduling policies, which avoid the starvation problem in process scheduling. At the end of this assignment, I learnt and was able to explain the advantages and disadvantages of the two new scheduling policies.

The default scheduler in Xinu schedules processes based on the highest priority. Starvation occurs when two or more processes that are eligible for execution have different priorities. The process with the higher priority gets to execute first, resulting in processes with lower priorities never getting any CPU time unless process with the higher priority ends.

The two scheduling policies that you need to implement, as described as follows, should address this problem. Note that for each of them, you need to consider how to handle the NULL process, so that this process is selected to run when and only when there are no other ready processes.

For Linux-like scheduling policies, the value of a valid process priority is an integer between 0 to 99, where 99 is the highest priority. 

The entire requirement document can be found at - https://people.engr.ncsu.edu/gjin2/Classes/501/Fall2019/assignments/PA1/pa1.html
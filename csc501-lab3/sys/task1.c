#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include "lock.h"

void semaphore1_test(int s1){
        kprintf("A\n");
        int x = wait(s1);
        kprintf("A: in Critical Section\n");
        sleep(1);
        kprintf("A: moving outside Critical Section.\n");
        signal(s1);
}

void semaphore2_test(int s2){
        kprintf("B before sleep\n");
        sleep(1);
        kprintf("B after waking up\n");
}

void semaphore3_test(int s3){
        kprintf("C\n");
        int x = wait(s3);
        kprintf("C: in Critical Section\n");
        sleep(1);
        kprintf("C: moving outside Critical Section.\n");
        signal(s3);
}

void lock1_test(int l1){
        kprintf("Process A\n");
        int x = lock(l1,WRITE,20);
        kprintf("A: Lock acquired.\n");
        sleep(1);
        kprintf("A: Releasing lock.\n");
        releaseall(1,l1);
}

void lock2_test(int l2){
        kprintf("B before sleep\n");
        sleep(3);
        kprintf("B after waking up\n");
}

void lock3_test(int l3){
        kprintf("Process C\n");
        int x = lock(l3,WRITE,20);
        kprintf("C: Lock acquired\n");
        sleep(1);
        kprintf("C: Releasing lock\n");
        releaseall(1,l3);
}

void semaphoreTester() {
        int semProcess = screate(1);
        int s1 = create(semaphore1_test,2000,20,"A",1,semProcess);
        int s2 = create(semaphore2_test,2000,30,"B",1,semProcess);
        int s3 = create(semaphore3_test,2000,40,"C",1,semProcess);

        kprintf("Starting A.\n");
        resume(s1);
        kprintf("Starting B.\n");
        resume(s2);
        sleep(1);
        kprintf("Starting C.\n");
        resume(s3);
        sleep(5);    
}

void lockTester() {
        int lockProcess = lcreate();
        int l1 = create(lock1_test,2000,20,"A",1,lockProcess);
        int l2 = create(lock2_test,2000,30,"B",1,lockProcess);
        int l3 = create(lock3_test,2000,40,"C",1,lockProcess);

        kprintf("Starting A.\n");
        resume(l1);
        kprintf("Starting B.\n");
        resume(l2);
        sleep(1);
        kprintf("Starting C.\n");
        resume(l3);
        sleep(5);
}

int main(){
        kprintf("Testing with semaphores.\n");
        semaphoreTester();

        kprintf("\nTesting with reader/writer locks using priority inversion.\n");
        lockTester();

        shutdown();
}

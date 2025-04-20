#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}





#ifndef PCB_H
#define PCB_H

#include <sys/types.h> // Needed for pid_t

typedef struct PCB
{
    int processID;                    // Logical ID (from processes.txt)
    pid_t processPID;                 // Actual OS-level PID from fork()

    int processPriority;              // For HPF (lower = higher priority)

    int arrivalTime;                  // Time process enters the system
    int runtime;                      // Total time needed to finish
    int remainingTime;                // Required for preemptive algorithms like SRTN

    int startTime;                    // When the process actually starts
    int finishTime;                   // When it ends
    int LastExecTime;                // Used in RR to calculate time slices
    int last_scheduled_time;

    int waitingTime;                 // Total time in ready queue
    int turnAroundTime;              // finishTime - arrivalTime
    float weightedTurnAroundTime;   // WTA = TA / runtime
} PCB;

#endif // PCB_H


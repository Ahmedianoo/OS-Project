#include <sys/types.h> // Needed for pid_t
#include "memory.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum algorithms
{
    HELPER,
    HPF,
    SRTN,
    RR
};

typedef struct PCB
{
    int processID;                 // Logical ID (from processes.txt)
    pid_t processPID, processPPID; // Actual OS-level PID from fork()

    bool isFirstRun;
    bool isStopped;

    int processPriority; // For HPF (lower = higher priority)

    int arrivalTime;   // Time process enters the system
    int runtime;       // Total time needed to finish
    int remainingTime; // Required for preemptive algorithms like SRTN

    int finished;

    int forked;

    int pStart;
    int startTime;    // When the process actually starts
    int finishTime;   // When it ends
    int LastExecTime; // Used in RR to calculate time slices
    int last_scheduled_time;

    int waitingTime;              // Total time in ready queue
    int turnAroundTime;           // finishTime - arrivalTime
    float weightedTurnAroundTime; // WTA = TA / runtime
    int memorysize;
    MemoryBlock *memPtr;
} PCB;

typedef struct msgbuff
{
    long mtype; // Message type (required by msgsnd)
    PCB data;   // The actual PCB to send
} msgbuff;

const char *algorithmToString(enum algorithms algo)
{
    switch (algo)
    {
    case HPF:
        return "1";
    case SRTN:
        return "2";
    case RR:
        return "3";
    default:
        return "0";
    }
}


int comparePCBForSRTN(const void* a, const void* b) {
    PCB* p1 = (PCB*)a;
    PCB* p2 = (PCB*)b;

    if (p1->arrivalTime != p2->arrivalTime)
        return p1->arrivalTime - p2->arrivalTime;

    // Same arrival time  use remaining time
    if (p1->remainingTime != p2->remainingTime)
        return p1->remainingTime - p2->remainingTime;

    // If both are still equal  use processID as final tie-breaker
    return p1->processID - p2->processID;
}
//A negative number if a < b (meaning a comes first)
//A positive number if a > b (meaning b comes first)
//Zero if they’re equal (no swap needed)
#include <sys/types.h> // Needed for pid_t

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

    int startTime;    // When the process actually starts
    int finishTime;   // When it ends
    int LastExecTime; // Used in RR to calculate time slices
    int last_scheduled_time;

    int waitingTime;              // Total time in ready queue
    int turnAroundTime;           // finishTime - arrivalTime
    float weightedTurnAroundTime; // WTA = TA / runtime
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
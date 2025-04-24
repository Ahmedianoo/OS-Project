#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <math.h>
#include <signal.h>
#include "headers.h"
#include "PCB.h"
#include "priority_queue.h"
#include "circularqueue.h"
FILE *logFile, *perfFile;
int msgq_id;
int cpuBusyTime = 0;
int totalWaiting = 0, totalProcesses = 0;
float totalWTA = 0;
float wtaArray[1000];
int wtaCount = 0;
int startTime = 0, finishTime = 0;
int fromPGenQId;
void InitComGentoScheduler()
{
    fromPGenQId = msgget(ProcessGenToSchedulerQKey, 0666 | IPC_CREAT);
    if (fromPGenQId == -1)
    {
        perror("process gen to scheduler msg Q");
    }
}
void writeLog(int time, PCB p, const char state) {
    fprintf(logFile, "At time %d process %d %s arr %d total %d remain %d wait %d",
            time, p.processID, state, p.arrivalTime, p.runtime, p.remainingTime, p.waitingTime);

    if (strcmp(state, "finished") == 0) {
        fprintf(logFile, " TA %d WTA %.2f", p.turnAroundTime, p.weightedTurnAroundTime);
    }

    fprintf(logFile, "\n");
    fflush(logFile);
}
void writePerformance() {
    float avgWaiting = (float)totalWaiting / totalProcesses;
    float avgWTA = totalWTA / totalProcesses;
    float stdWTA = 0;

    for (int i = 0; i < wtaCount; i++)
        stdWTA += pow(wtaArray[i] - avgWTA, 2);

    stdWTA = sqrt(stdWTA / totalProcesses);
    float utilization = ((float)cpuBusyTime / (finishTime - startTime))* 100;

    fprintf(perfFile, "CPU utilization = %.2f%%\n", utilization);
    fprintf(perfFile, "Avg WTA = %.2f\n", avgWTA);
    fprintf(perfFile, "Avg Waiting = %.2f\n", avgWaiting);
    fprintf(perfFile, "Std WTA = %.2f\n", stdWTA);

    fclose(perfFile);
}   
    void cleanup() {
        msgctl(msgq_id, IPC_RMID, NULL);
        destroyClk(false);
        fclose(logFile);
    }
    void runHPF() {
        PriorityQueue *readyQueue = createQueue();
        int processRunning = 0;
        pid_t currentPID = -1;
        PCB currentProcess;
    
        while (1) {
            int now = getClk();
    
            // Receive new processes
            msgbuff msg;
            while (msgrcv(msgq_id, &msg, sizeof(PCB), 0, IPC_NOWAIT) != -1) {
                enqueue(readyQueue, msg.process);
            }
    
            // If idle, start the next process
            if (!processRunning && !isEmpty(readyQueue)) {
                currentProcess = dequeue(readyQueue);
                currentProcess.startTime = now;
                currentProcess.waitingTime = now - currentProcess.arrivalTime;
                currentProcess.remainingTime = currentProcess.runtime;
    
                writeLog(now, currentProcess, "started");
    
                currentPID = fork();
                if (currentPID == 0) {
                    char runtimeStr[10];
                    sprintf(runtimeStr, "%d", currentProcess.runtime);
                    execl("./process.out", "process.out", runtimeStr, NULL);
                    perror("Failed to exec process");
                    exit(1);
                }
    
                cpuBusyTime += currentProcess.runtime;
                processRunning = 1;
    
                if (startTime == 0)
                    startTime = now;
            }
    
            // Check if current process is done
            if (processRunning) {
                int status;
                pid_t result = waitpid(currentPID, &status, WNOHANG);
                if (result == currentPID) {
                    int endTime = getClk();
                    currentProcess.finishTime = endTime;
                    currentProcess.turnAroundTime = endTime - currentProcess.arrivalTime;
                    currentProcess.weightedTurnAroundTime = (float)currentProcess.turnAroundTime / currentProcess.runtime;
    
                    writeLog(endTime, currentProcess, "finished");
    
                    totalWTA += currentProcess.weightedTurnAroundTime;
                    totalWaiting += currentProcess.waitingTime;
                    wtaArray[wtaCount++] = currentProcess.weightedTurnAroundTime;
                    totalProcesses++;
    
                    finishTime = endTime;
                    processRunning = 0;
                }
            }
    
            sleep(1);
        }
    }
    void runSRTN() {
        printf("[Not implemented] SRTN will go here.\n");
        exit(0);
    }
    
    void runRR(int quantum) {
        printf("[Not implemented] RR with quantum %d will go here.\n", quantum);
        exit(0);
    }
    
struct CircularQueue myQ;
msgbuff RecieveProcess()
{
    struct msgbuff myMsg;
    msgrcv(fromPGenQId, &myMsg, sizeof(myMsg.data), getpid() % 10000, !IPC_NOWAIT);
    return myMsg;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: scheduler.out <algorithm_number> [quantum_for_RR]\n");
        return 1;
    }
    int algorithm = atoi(argv[1]);
    int quantum = (argc >= 3) ? atoi(argv[2]) : 0;
    signal(SIGINT, cleanup);
    setvbuf(stdout, NULL, _IONBF, 0);
    
    initClk();
    InitComGentoScheduler();

    initQueue(&myQ);
    struct msgbuff myMsg;
    msgq_id = msgget(MSGKEY, 0666);
    if (msgq_id == -1) {
        perror("Scheduler: failed to access message queue");
        exit(1);
    }
    logFile = fopen("scheduler.log", "w");
    perfFile = fopen("scheduler.perf", "w");
    if (!logFile || !perfFile) {
        perror("Failed to open log/perf files");
        exit(1);
    }

    switch (algorithm) {
        case 1:
            runHPF();
            break;
        case 2:
            runSRTN();
            break;
        case 3:
            runRR(quantum);
            break;
        default:
            fprintf(stderr, "Invalid algorithm. Use 1 for HPF, 2 for SRTN, 3 for RR.\n");
            break;
    }
    writePerformance();
    cleanup();
    return 0;
    while (true)
    {

        myMsg = RecieveProcess();
        printf("\n recieved process with id: %d\n", myMsg.data.processID);
    }
    // TODO implement the scheduler :)
    // Round Robin

    // upon termination release the clock resources
}

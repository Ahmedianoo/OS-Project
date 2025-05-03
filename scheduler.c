#include "headers.h"
#include "SRTNQueue.h"
#include "circularqueue.h"

enum algorithms algorithm;
PCB runningProcess;
PCB *currentProcess;
// PCB *currentProcess1;

void processFinished_handler(int signum)
{

    int stat_loc;

    int pid = wait(&stat_loc);
    // printf("i am %d my status is %d", pid, WEXITSTATUS(stat_loc));
    if (pid == -1)
    {
        perror("Error while waiting for a child process");
    }
    else if (WIFEXITED(stat_loc))
    {
        int finishclk = WEXITSTATUS(stat_loc);
        // if (algorithm == RR)
        // {

        printf("process #%d has finished at %d.\n", pid, finishclk);
        // }
        // else
        // {

        //     runningProcess.finishTime = getClk();
        //     printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
        // }
        // killpg(getpgrp(), SIGKILL);
    }

    signal(SIGUSR1, processFinished_handler);
}

int fromPGenQId;
void InitComGentoScheduler()
{
    fromPGenQId = msgget(ProcessGenToSchedulerQKey, 0666 | IPC_CREAT);
    if (fromPGenQId == -1)
    {
        perror("process gen to scheduler msg Q");
    }
}

msgbuff RecieveProcess(bool *success)
{
    struct msgbuff myMsg;
    int r = msgrcv(fromPGenQId, &myMsg, sizeof(myMsg) - sizeof(long), getpid() % 10000, IPC_NOWAIT);

    if (r != -1)
    {
        *success = true;
        return myMsg;
        // suggested!!//return myMsg.data;//and change the return type to PCB i think would be better(Ali)
        // i also suggest sending the total number of process from the process manager that would help a lot :)
        // it would make you know that you have finished and not recieve prcesses again and break from the infinte loop
    }
    else
    {
        *success = false;
        struct msgbuff empty = {0};
        return empty;
    }
}

void SRTN_algo()
{
    char remaining_str[10];
    struct msgbuff myMsg;
    PCB recProcess;
    runningProcess;
    bool success = 0;
    PriorityQueueSRTN *readyQueue = createQueue();

    while (success == 0 || myMsg.data.arrivalTime > getClk())
    {

        myMsg = RecieveProcess(&success);
    }

    runningProcess = myMsg.data;
    printf("\n recieved process with id: %d\n", runningProcess.processID);

    bool first = 1;

    int pStart;

    while (true)
    {

        if (first)
        {

            first = 0;
            if (runningProcess.remainingTime > 0)
            {

                sprintf(remaining_str, "%d", runningProcess.remainingTime);
                runningProcess.processPID = fork();
                runningProcess.startTime = getClk();
                pStart = runningProcess.startTime;

                if (runningProcess.processPID == 0)
                {
                    execl("./process.out", "process", remaining_str, NULL);
                    perror("execl failed");
                    exit(1);
                }
            }
        }
        else
        {
            myMsg = RecieveProcess(&success);
            if (success && myMsg.data.arrivalTime <= getClk())
            {

                // printf("i am here in the srtn\n");
                recProcess = myMsg.data;
                enqueue(readyQueue, recProcess);
                // printf("i am here after the enqueue with id %d\n", recProcess.processID);
                if (readyQueue->head->data.remainingTime < runningProcess.remainingTime || runningProcess.remainingTime == 0)
                {
                    // printf("i am here after the enqueue with id %d, i have remaining time %d, the pstart %d\n", recProcess.processID, recProcess.remainingTime, pStart);
                    int tempclk = getClk();
                    // rintf("i am the running process %d\n", runningProcess.processID);

                    if (runningProcess.finishTime == -1)
                    {
                        runningProcess.remainingTime = runningProcess.remainingTime - (tempclk - pStart);
                        runningProcess.last_scheduled_time = tempclk;
                        kill(runningProcess.processID, SIGSTOP);
                        enqueue(readyQueue, runningProcess);
                        runningProcess = dequeue(readyQueue);
                    }
                    else
                    {

                        runningProcess = dequeue(readyQueue);
                    }

                    if (runningProcess.processPID == -1)
                    { // check if there is a better way to do this check

                        printf("\n recieved process with id: %d\n", runningProcess.processID);

                        runningProcess.processPID = fork();
                        runningProcess.startTime = getClk();
                        pStart = runningProcess.startTime;
                        sprintf(remaining_str, "%d", runningProcess.remainingTime);

                        if (runningProcess.processPID == 0)
                        {
                            execl("./process.out", "process", remaining_str, NULL);
                            perror("execl failed");
                            exit(1);
                        }
                    }
                    else
                    {
                        kill(runningProcess.processPID, SIGCONT);
                        pStart = getClk();
                    }
                }
            }
        }
    }
    return;
}

void RR_algo(int Quantum)
{
    // int Quantum = 1;
    int processesCount = 0;
    int startQuantum = 100;
    int execDuration = 0;
    bool pGotRem = false;

    bool existsRunning = false;
    struct CircularQueue myQ;
    initQueue(&myQ);

    bool success = 0;
    while (true)
    {
        for (int i = 0; i < 100; i++)
        {
            struct msgbuff myMsg = RecieveProcess(&success);
            if (success)
            {
                printf("\n recieved process with id: %d,arrival time : %d, at clock: %d\n", myMsg.data.processID, myMsg.data.arrivalTime, getClk());
                enqueueCirc(&myQ, myMsg.data);
                processesCount++;
                // printQueue(&myQ);

                break;
            }
        }
        if (processesCount > 0)
        {
            currentProcess = peekCurrent(&myQ);

            if (!existsRunning && processesCount > 1 && !pGotRem)
            {
                printf("entered exchange block\n");
                printQueue(&myQ);
                rotate(&myQ); // need to be checked
                printQueue(&myQ);

                pGotRem = false;
                currentProcess = peekCurrent(&myQ);
            }
            else
            {

                pGotRem = false;
            }
            if (!existsRunning)
            {
                if (currentProcess->isFirstRun)
                {
                    currentProcess->startTime = getClk();
                    printf("i :%d started at %d\n", currentProcess->processID, currentProcess->startTime);
                    currentProcess->isFirstRun = false;

                    char remaining_str[10];
                    sprintf(remaining_str, "%d", currentProcess->remainingTime);

                    currentProcess->processPID = fork();
                    if (currentProcess->processPID == 0)
                    {
                        execl("./process", "process", remaining_str, NULL);
                        perror("execl failed: check file name 1");
                    }
                }
                else if (!existsRunning)
                {
                    printf("\ni am %d continuing at %d \n", currentProcess->processID, getClk());
                    printQueue(&myQ);
                    kill(currentProcess->processPID, SIGCONT);

                    // printf("clk ro be signale %d now clk is %d \n", startQuantum + execDuration, getClk());
                }
                existsRunning = true;
                startQuantum = getClk();
                execDuration = MIN(Quantum, currentProcess->remainingTime);
                printf("execution duration %d\n", execDuration);
                currentProcess->remainingTime -= execDuration;
            }
            int current_time = getClk();
            while (existsRunning)
            {

                if ((startQuantum + execDuration) <= getClk())
                {

                    if (currentProcess->remainingTime <= 0)
                    {
                        removeCurrent(&myQ);
                        pGotRem = true;
                        printQueue(&myQ);
                        processesCount--;
                    }
                    else
                    {
                        kill(currentProcess->processPID, SIGSTOP);
                        printf("\n ha U stopped n%d at %d\n", currentProcess->processPID, getClk());
                    }
                    existsRunning = false;
                    // rotate(&myQ);
                }

                if (getClk() == current_time + 1)
                {
                    struct msgbuff myMsg = RecieveProcess(&success);
                    if (success)
                    {

                        printf("\n recieved process with id: %d,arrival time : %d, at clock: %d\n", myMsg.data.processID, myMsg.data.arrivalTime, getClk());
                        enqueueCirc(&myQ, myMsg.data);
                        processesCount++;
                        // printQueue(&myQ);

                        // break;
                    }
                    current_time = getClk();
                }
            }

            // if (/* generatorDone!! && */ processesCount == 0)
            // {
            //     break;
            // }
        }
    }
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\nhello\n");
    initClk();
    InitComGentoScheduler();
    signal(SIGUSR1, processFinished_handler);
    int Quantum = atoi(argv[2]);
    algorithm = atoi(argv[1]);
    while (true)
    {

        printf("\n recieved the algorithm: %d", algorithm);

        switch (algorithm)
        {
        case HPF:
            printf("\n processing with HPF...");
            // call your function here
            break;

        case SRTN:
            printf("\n processing with SRTN...");
            SRTN_algo();

            break;

        case RR:
            printf("\n processing with RR...");
            printf("with %d Quantum", Quantum);
            RR_algo(Quantum);
            break;

        default:
            break;
        }
    }
    // TODO implement the scheduler :)
    // Round Robin

    // upon termination release the clock resources
}

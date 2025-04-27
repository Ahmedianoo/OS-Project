#include "headers.h"
#include "SRTNQueue.h"
#include "circularqueue.h"

PCB runningProcess;

void processFinished_handler(int signum)
{

    int stat_loc;

    int pid = wait(&stat_loc);
    if (pid == -1)
    {
        perror("Error while waiting for a child process");
    }
    else if (WIFEXITED(stat_loc))
    {
        int index = WEXITSTATUS(stat_loc);
        runningProcess.finishTime = getClk();
        printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
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
                pStart = getClk();

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

                recProcess = myMsg.data;
                enqueue(readyQueue, recProcess);
                if (readyQueue->head->data.remainingTime < runningProcess.remainingTime || runningProcess.remainingTime == 0)
                {

                    runningProcess.remainingTime = runningProcess.remainingTime - (getClk() - pStart);
                    runningProcess.last_scheduled_time = getClk();
                    kill(runningProcess.processID, SIGSTOP);
                    enqueue(readyQueue, runningProcess);
                    runningProcess = dequeue(readyQueue);

                    if (runningProcess.processPID == -1)
                    { // check if there is a better way to do this check

                        printf("\n recieved process with id: %d\n", runningProcess.processID);

                        runningProcess.processPID = fork();
                        runningProcess.startTime = getClk();
                        pStart = getClk();
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

void RR_algo()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    int Quantum = 1;
    char remaining_str[10];
    int processesCount = 0;
    int startQuantum = 0;
    struct CircularQueue myQ;
    initQueue(&myQ);
    PCB *currentProcess;
    bool success = 0;
    // PCB newProcess = RecieveProcess(success);
    while (true)
    {

        if (true)
        {
            struct msgbuff myMsg = RecieveProcess(&success);
            if (success)
            {
                printf("\n recieved process with id: %d\n", myMsg.data.processID);
                enqueueCirc(&myQ, myMsg.data);
                sprintf(remaining_str, "%d", currentProcess->remainingTime);
                processesCount++;
            }
        }
        if (processesCount > 0)
        {
            printQueue(&myQ);
            currentProcess = peekCurrent(&myQ);
            if (currentProcess->isFirstRun)
            {
                currentProcess->startTime = getClk();
                currentProcess->isFirstRun = false;
                currentProcess->processPID = fork();
                if (currentProcess->processPID == 0)
                {
                    execl("./process", "process", remaining_str, NULL);
                    perror("execl failed: check file name");
                    exit(-1);
                }
            }
            else
            {
                kill(currentProcess->processPID, SIGCONT);
            }
            startQuantum = getClk();
            if (getClk() - startQuantum >= Quantum)
            {
                kill(runningProcess.processID, SIGSTOP);
            }
            rotate(&myQ);
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

    enum algorithms algorithm;

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
            RR_algo();
            break;

        default:
            break;
        }
    }
    // TODO implement the scheduler :)
    // Round Robin

    // upon termination release the clock resources
}

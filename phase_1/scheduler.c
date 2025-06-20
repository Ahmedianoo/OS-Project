#include "headers.h"
#include "SRTNQueue.h"
#include "circularqueue.h"
#include <math.h>

enum algorithms algorithm;
PCB runningProcess;
bool contSRTN = false;
int noOfprocesses;
int noOfRec;

PCB *currentProcess;
bool childFinished;
FILE *logFile;
FILE *perfFile;

// Process stats
int totalWaiting = 0; //accumulation of the waiting time.. start - arrival of each process 
int totalProcesses = 0; //number of total processes----
float totalWTA = 0.0;   
float wtaArray[1000];  // Adjust size as needed
int wtaCount = 0;

// CPU time tracking
int cpuBusyTime = 0; //the running time of the processes
int startTime = -1;  // start time of the first process---
int finishTime = -1; // the finish time of the last process----   

void writeLog(int time, PCB p, const char *state)
{
    fprintf(logFile, "At time %d process %d %s arr %d total %d remain %d wait %d",
            time, p.processID, state, p.arrivalTime, p.runtime, p.remainingTime, p.waitingTime);

    if (strcmp(state, "finished") == 0)
    {
        fprintf(logFile, " TA %d WTA %.2f", p.turnAroundTime, p.weightedTurnAroundTime);
    }

    fprintf(logFile, "\n");
    fflush(logFile);
}
void writePerformance()
{
    float avgWaiting = (float)totalWaiting / totalProcesses;
    float avgWTA = totalWTA / totalProcesses;
    float stdWTA = 0.0f;

    for (int i = 0; i < wtaCount; i++)
        stdWTA += pow(wtaArray[i] - avgWTA, 2);

    stdWTA = sqrt(stdWTA / totalProcesses);
    float utilization = ((float)cpuBusyTime / (finishTime - startTime)) * 100;

    fprintf(perfFile, "CPU utilization = %.2f%%\n", utilization);
    fprintf(perfFile, "Avg WTA = %.2f\n", avgWTA);
    fprintf(perfFile, "Avg Waiting = %.2f\n", avgWaiting);
    fprintf(perfFile, "Std WTA = %.2f\n", stdWTA);
    fflush(perfFile);
}

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
        int finish = WEXITSTATUS(stat_loc);
        if (algorithm == RR)
        {

            noOfprocesses--;
            printf("process #%d has finished at %d.\n", pid, finish);
        }
        else if (algorithm == SRTN)
        {
            runningProcess.finishTime = finish;
            //runningProcess.finishTime = getClk();
            runningProcess.turnAroundTime = runningProcess.finishTime - runningProcess.arrivalTime;
            runningProcess.weightedTurnAroundTime =(float)runningProcess.turnAroundTime / runningProcess.runtime;
    
           
            totalWaiting += runningProcess.waitingTime;///
            totalWTA += runningProcess.weightedTurnAroundTime;
            wtaArray[wtaCount++] = runningProcess.weightedTurnAroundTime;
            cpuBusyTime += runningProcess.runtime;
    
             writeLog(runningProcess.finishTime, runningProcess, "finished");
             runningProcess.finished = 1;
             runningProcess.remainingTime = 0;
             noOfprocesses--;
             noOfRec--;
             printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
             contSRTN = true;
        }
        else if (algorithm == HPF)
        {

            runningProcess.finishTime = getClk();
            printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
            childFinished = true;
        }
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
    runningProcess.startTime = getClk();
    //runningProcess.waitingTime = runningProcess.startTime - runningProcess.arrivalTime;
    runningProcess.pStart = runningProcess.startTime;
    startTime = runningProcess.startTime;
    // printf("ahmedhamdasokarziada");
    printf("\n recieved process with id: %d\n", runningProcess.processID);
    noOfRec++;

    bool first = 1;
  

    //finish-arrr-runtime
 


    while (true)
    {

        if (first)
        {

            first = 0;
            if (runningProcess.remainingTime > 0)
            {

                // printf("%d\n", runningProcess.processPID);
                sprintf(remaining_str, "%d", runningProcess.remainingTime);

                runningProcess.processPID = fork();
                if (runningProcess.processPID == -1)
                {
                    perror("forking failed");
                    exit(1);
                }

                if (runningProcess.processPID == 0)
                {
                    execl("./process.out", "process", remaining_str, NULL);
                    perror("execl failed");
                    exit(1);
                }
                else
                {

                    runningProcess.forked = 1;
                    writeLog(runningProcess.startTime, runningProcess, "started");
                }
            }
        }
        else
        {


            // if (runningProcess.finished == 1 && runningProcess.forked == 1) {

            //     //runningProcess.waitingTime=0;

            // }



            myMsg = RecieveProcess(&success);
            if (success)
            {
                noOfRec++;
                printf("number of recieved and not finished %d\n", noOfRec);
            }
            // printf("%d\n", noOfRec);
            // if(noOfRec == 0){
            //     printf("i am zero now no of processes");
            // }
            // count of received processes and check for them may you
            //&& noOfRec > 0
            // if(noOfRec == 0){

            // }

            if(noOfprocesses > 0 && noOfRec == 0){
                
                continue;
            }

            if(noOfprocesses == 0){

                finishTime = getClk();
                writePerformance();
                destroyQueue(readyQueue);
                printf("program is finished bye!\n");
                break;
            }
            else if ((contSRTN || (success && myMsg.data.arrivalTime <= getClk())))
            { // there is an issue with the condition as we use the logic built on success
                contSRTN = false;

                // printf("i am here in the srtn\n");
                if (success)
                {
                    printf("i have succeed\n");
                    recProcess = myMsg.data;
                    recProcess.last_scheduled_time=getClk();
                    enqueue(readyQueue, recProcess);
                }

                // printf("i am here after the enqueue with id %d\n", readyQueue->head->data.remainingTime);
                if (runningProcess.remainingTime == 0 || readyQueue->head->data.remainingTime < runningProcess.remainingTime)
                {
                    // printf("i am here after the enqueue with id %d, i have remaining time %d, the pstart\n", recProcess.processID, recProcess.remainingTime);
                    int tempclk = getClk();

                    // printf("i am the running process %d\n", runningProcess.processID);
                    if (runningProcess.finishTime == -1)
                    {
                        // printf("remaining time %d %d\n", runningProcess.processPID, runningProcess.remainingTime);
                        runningProcess.remainingTime = runningProcess.remainingTime - (tempclk - runningProcess.pStart);
                        // printf("remaining time %d %d\n", runningProcess.processPID, runningProcess.remainingTime);
                        runningProcess.pStart = tempclk;
                        runningProcess.last_scheduled_time = tempclk;
                        // printf("before sigstop, i am the running process %d\n", runningProcess.processPID);
                        enqueue(readyQueue, runningProcess);
                        kill(runningProcess.processPID, SIGSTOP);
                        writeLog(tempclk,runningProcess,"stopped");
                        //runningProcess.waitingTime=tempclk-runningProcess.last_scheduled_time+runningProcess.waitingTime;
                        runningProcess.last_scheduled_time=tempclk;
                        runningProcess = dequeue(readyQueue);
                    }
                    else
                    {

                        // printf("i am the running process %d\n", runningProcess.processID);
                        runningProcess = dequeue(readyQueue);
                        // printf("i am the running process %d\n", runningProcess.processPID);
                        // printf("i am the running process %d\n", runningProcess.processID);
                    }
                    // printf("i am the running process %d\n", runningProcess.processID);
                    // printf("%d\n", runningProcess.processPID);

                    if (runningProcess.forked == -1)
                    { // check if there is a better way to do this check

                        printf("\n recieved process with id: %d\n", runningProcess.processID);

                        runningProcess.startTime = getClk();

                        runningProcess.pStart = runningProcess.startTime;

                        sprintf(remaining_str, "%d", runningProcess.remainingTime);
                        runningProcess.processPID = fork();
                        if (runningProcess.processPID == -1)
                        {
                            perror("forking failed");
                            exit(1);
                        }


    
                        if (runningProcess.processPID == 0) {
                            execl("./process.out", "process", remaining_str,NULL);
                            perror("execl failed"); 
                            exit(1);
                        }
                        else
                        {

                            runningProcess.forked = 1;
                            //runningProcess.waitingTime = runningProcess.startTime - runningProcess.arrivalTime;
                            runningProcess.waitingTime=tempclk-runningProcess.last_scheduled_time+runningProcess.waitingTime;
                            writeLog(runningProcess.startTime, runningProcess, "started");
                        }
                        
                    }else if (runningProcess.processID != 0){
                        printf("i will cont. now: %d\n", runningProcess.processPID);
                        contSRTN = false;
                        kill(runningProcess.processPID, SIGCONT);
                        runningProcess.pStart = getClk();
                        runningProcess.waitingTime=runningProcess.pStart-runningProcess.last_scheduled_time+runningProcess.waitingTime;
                        writeLog(runningProcess.pStart, runningProcess, "continue");
                    }
                }
            }
        }

        // usleep(1000);
    }

    // waitclk();
    return;
}

void HPF_algo() { 
    
    PriorityQueue* readyQueue = createQueueHPF();
     if (!readyQueue) { 
        perror("Failed to create HPF queue"); 
        destroyClk(true); 
        exit(1); 
    }

    signal(SIGUSR1, processFinished_handler); // register finish signal

    runningProcess;
    msgbuff msg;
    bool success = false;
    bool cpuFree = true;
    char remaining_str[10];
    int totalCount = noOfprocesses;
    int finishedCount = 0;
    printf("Total processes: %d\n", totalCount);

    // Wait for first valid process
    while (1)
    {
        msg = RecieveProcess(&success);
        if (success && msg.data.arrivalTime <= getClk())
        {
            msg.data.remainingTime = msg.data.runtime;
            enqueueHPF(readyQueue, msg.data);
            break;
        }
        // usleep(500);
    }

    while (1)
    {
        int now = getClk();

        // Step 1: Receive all new processes
        do
        {
            msg = RecieveProcess(&success);
            if (success && msg.data.arrivalTime <= now)
            {
                msg.data.remainingTime = msg.data.runtime;
                enqueueHPF(readyQueue, msg.data);
                printf("Time %d: Enqueued process ID=%d, priority=%d\n",
                       now, msg.data.processID, msg.data.processPriority);
            }
        } while (success);

        // Step 2: If CPU is idle, schedule next
        if (cpuFree && !isEmptyHPF(readyQueue))
        {

            runningProcess = dequeueHPF(readyQueue);
            sprintf(remaining_str, "%d", runningProcess.remainingTime);

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("fork failed");
                destroyClk(true);
                exit(1);
            }
            else if (pid == 0)
            {
                execl("./process.out", "process", remaining_str, NULL);
                perror("execl failed");
                destroyClk(false);
                exit(1);
            }

            runningProcess.processPID = pid;
            runningProcess.startTime = getClk();
            runningProcess.waitingTime = runningProcess.startTime - runningProcess.arrivalTime;
            writeLog(runningProcess.startTime, runningProcess, "started");
            if (startTime == -1)
            {
                startTime = runningProcess.startTime;
            }
            cpuFree = false;
            childFinished = 0;
        }

        // Step 3: If child finished via signal
        if (childFinished && !cpuFree)
        {
            runningProcess.finishTime = getClk();
            runningProcess.turnAroundTime = runningProcess.finishTime - runningProcess.arrivalTime;
            runningProcess.weightedTurnAroundTime = (float)runningProcess.turnAroundTime / runningProcess.runtime;
            runningProcess.remainingTime = 0;

            totalWaiting += runningProcess.waitingTime;
            totalWTA += runningProcess.weightedTurnAroundTime;
            wtaArray[wtaCount++] = runningProcess.weightedTurnAroundTime;
            cpuBusyTime += runningProcess.runtime;

            writeLog(runningProcess.finishTime, runningProcess, "finished");
            runningProcess.waitingTime = 0;
            finishedCount++;
            cpuFree = true;
            childFinished = 0;
        }

        // Step 4: Exit when done
        if (finishedCount == totalCount && isEmptyHPF(readyQueue) && cpuFree)
        {
            totalProcesses = totalCount;
            finishTime = getClk();
            break;
        }

        // usleep(500); // avoid busy waiting
    }
    writePerformance();
    destroyQueueHPF(readyQueue);
    printf("HPF Scheduling finished.\n");
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
                myMsg.data.last_scheduled_time=getClk();
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
                    //---------------------- Some Logs -----------------------------------------
                    currentProcess->startTime = getClk();
                    //currentProcess->waitingTime = currentProcess->startTime-currentProcess->last_scheduled_time+currentProcess->waitingTime;
                    writeLog(currentProcess->startTime, *currentProcess, "started");
                    //---------------------- Some Logs -----------------------------------------

                    printf("i :%d started at %d\n", currentProcess->processID, currentProcess->startTime);
                    currentProcess->isFirstRun = false;

                    char remaining_str[10];
                    sprintf(remaining_str, "%d", currentProcess->remainingTime);

                    currentProcess->processPID = fork();
                    if (currentProcess->processPID == 0)
                    {
                        execl("./process.out", "process", remaining_str, NULL);
                        perror("execl failed: check file name");
                        // exit(-1);
                    }
                }
                else if (!existsRunning)
                {
                    printf("\ni am %d continuing at %d \n", currentProcess->processID, getClk());
                    printQueue(&myQ);
                    kill(currentProcess->processPID, SIGCONT);
                    currentProcess->waitingTime=getClk()-currentProcess->last_scheduled_time+currentProcess->waitingTime;
                    writeLog(getClk(), *currentProcess, "Continues");
                    currentProcess->last_scheduled_time=getClk();
                    // printf("clk ro be signale %d now clk is %d \n", startQuantum + execDuration, getClk());
                }
                if (startTime == -1)
                {
                    startTime = runningProcess.startTime;
                }
                existsRunning = true;
                startQuantum = getClk();
                currentProcess->waitingTime+=startQuantum-currentProcess->last_scheduled_time;
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
                        //---------------------- Some Logs -----------------------------------------
                        currentProcess->finishTime = getClk();
                        currentProcess->turnAroundTime = currentProcess->finishTime - currentProcess->arrivalTime;
                        currentProcess->weightedTurnAroundTime = (float)currentProcess->turnAroundTime / currentProcess->runtime;
                        currentProcess->remainingTime = 0;

                        totalWaiting += currentProcess->waitingTime;
                        totalWTA += currentProcess->weightedTurnAroundTime;
                        wtaArray[wtaCount++] = currentProcess->weightedTurnAroundTime;
                        cpuBusyTime += currentProcess->runtime;
                        writeLog(currentProcess->finishTime, *currentProcess, "finished");
                        //---------------------- Some Logs -----------------------------------------
                        currentProcess->last_scheduled_time=getClk();
                        removeCurrent(&myQ);
                        pGotRem = true;
                        printQueue(&myQ);
                        processesCount--;
                    }
                    else
                    {
                        kill(currentProcess->processPID, SIGSTOP);
                        printf("\n ha U stopped n%d at %d\n", currentProcess->processPID, getClk());
                        currentProcess->last_scheduled_time=getClk();
                        writeLog(getClk(), *currentProcess, "Stopped");
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
                        myMsg.data.last_scheduled_time=getClk();
                        enqueueCirc(&myQ, myMsg.data);
                        processesCount++;
                        // printQueue(&myQ);

                        // break;
                    }
                    current_time = getClk();
                }
            }
        }
        else if (noOfprocesses == 0)
        {
            finishTime = getClk();
            writePerformance();
            printf("bye!!\n");
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\nhello\n");

    logFile = fopen("scheduler.log", "w");
    perfFile = fopen("scheduler.perf", "w");

    if (!logFile || !perfFile)
    {
        perror("Error opening log or performance file");
        return 1;
    }

    initClk();
    InitComGentoScheduler();
    signal(SIGUSR1, processFinished_handler);
    noOfprocesses = atoi(argv[2]);

    // enum algorithms algorithm;
    int Quantum = atoi(argv[3]);
    algorithm = atoi(argv[1]);
    // while (true)
    // {

    printf("\n recieved the algorithm: %d", algorithm);

    switch (algorithm)
    {
    case HPF:
        printf("\n processing with HPF...");
        HPF_algo();
        break;

        case SRTN:
            printf("\n processing with SRTN...");
            totalProcesses = noOfprocesses;
            SRTN_algo();
            break;

    case RR:
        printf("\n processing with RR...");
        printf("with %d Quantum", Quantum);
        totalProcesses = noOfprocesses;
        RR_algo(Quantum);
        break;

    default:
        break;
    }
    // }

    fclose(logFile);
    fclose(perfFile);
    // TODO implement the scheduler :)
    // Round Robin
    destroyClk(false);

    return 0;

    // upon termination release the clock resources
}

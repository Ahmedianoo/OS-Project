#include "headers.h"
#include "SRTNQueue.h"
#include "circularqueue.h"

enum algorithms algorithm;
PCB runningProcess;
PCB *currentProcess;
bool childFinished;

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
        if (algorithm == RR)
        {
            currentProcess->finishTime = getClk();
            printf("process #%d has started at time %d and finished at %d.\n", pid, currentProcess->startTime, currentProcess->finishTime);
        }
        else if(algorithm == SRTN)
        {
            runningProcess.finishTime = getClk();
            printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
        }
        else if(algorithm == HPF)
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

    while (!success == 0 || myMsg.data.arrivalTime > getClk())
    {

        myMsg = RecieveProcess(&success);
    }

    runningProcess = myMsg.data;
    printf("\n recieved process with id: %d\n", runningProcess.processID);

    bool first = 1;

    int pStart;


    while(true){
        
        if(first){

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
            if(success && myMsg.data.arrivalTime <= getClk()){

                //printf("i am here in the srtn\n");
                recProcess = myMsg.data;
                enqueue(readyQueue, recProcess);
                //printf("i am here after the enqueue with id %d\n", recProcess.processID);
                if(readyQueue->head->data.remainingTime < runningProcess.remainingTime || runningProcess.remainingTime == 0){
                    //printf("i am here after the enqueue with id %d, i have remaining time %d, the pstart %d\n", recProcess.processID, recProcess.remainingTime, pStart);
                    int tempclk = getClk();
                    //rintf("i am the running process %d\n", runningProcess.processID);


                    if(runningProcess.finishTime ==  -1){
                        runningProcess.remainingTime = runningProcess.remainingTime - (tempclk - pStart);
                        runningProcess.last_scheduled_time = tempclk;
                        kill(runningProcess.processID, SIGSTOP);
                        enqueue(readyQueue, runningProcess);
                        runningProcess = dequeue(readyQueue);

                    }else {


                        runningProcess = dequeue(readyQueue);


                    }

                    
    
                    if(runningProcess.processPID == -1){// check if there is a better way to do this check
    
    

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

void HPF_algo() { 
    
    PriorityQueue* readyQueue = createQueueHPF();
     if (!readyQueue) { 
        perror("Failed to create HPF queue"); 
        destroyClk(true); 
        exit(1); }

signal(SIGUSR1, processFinished_handler); // register finish signal


PCB runningProcess = {0};
msgbuff msg;
bool success = false;
bool cpuIdle = true;
char remaining_str[10];
int totalCount = 0;
int finishedCount = 0;

// Wait for first valid process
while (1) {
    msg = RecieveProcess(&success);
    if (success && msg.data.arrivalTime <= getClk()) {
        msg.data.remainingTime = msg.data.runtime;
        enqueueHPF(readyQueue, msg.data);
        totalCount++;
        break;
    }
    usleep(500);
}

while (1) {
    int now = getClk();

    // Step 1: Receive all new processes
    msg = RecieveProcess(&success);
    while (success) {
        if (msg.data.arrivalTime <= now) {
            msg.data.remainingTime = msg.data.runtime;
            enqueueHPF(readyQueue, msg.data);
            totalCount++;
            printf("Time %d: Enqueued process ID=%d, priority=%d\n",
                   now, msg.data.processID, msg.data.processPriority);
        }
        msg = RecieveProcess(&success); // try next
    }
    

    // Step 2: If CPU is idle, schedule next
    if (cpuIdle && !isEmptyHPF(readyQueue)) {
    
        runningProcess = dequeueHPF(readyQueue);
        sprintf(remaining_str, "%d", runningProcess.remainingTime);
        

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            destroyClk(true);
            exit(1);
        } else if (pid == 0) {
            execl("./process.out", "process", remaining_str, NULL); //problem here
            perror("execl failed");
            destroyClk(false);
            exit(1);
        }

        runningProcess.processPID = pid;
        runningProcess.startTime = getClk();
        printf("Time %d: Started process ID=%d (priority=%d)\n",
               runningProcess.startTime,
               runningProcess.processID,
               runningProcess.processPriority);
        cpuIdle = false;///
        childFinished = 0; // reset flag
       
    }

    // Step 3: If child finished via signal
    if (childFinished && !cpuIdle) {
        runningProcess.finishTime = getClk();
        runningProcess.turnAroundTime = runningProcess.finishTime - runningProcess.arrivalTime;
        runningProcess.weightedTurnAroundTime =
            (float)runningProcess.turnAroundTime / runningProcess.runtime;
        runningProcess.remainingTime = 0;

        printf("Time %d: Finished process ID= %d → TA= %d, WTA= %.2f\n",
               runningProcess.finishTime,
               runningProcess.processID,
               runningProcess.turnAroundTime,
               runningProcess.weightedTurnAroundTime);

        // Optional: log to file here
        finishedCount++;
        cpuIdle = true;
        childFinished = 0;
    }

    // Step 4: Exit when done
    if (finishedCount == totalCount && isEmptyHPF(readyQueue) && cpuIdle) {
        break;
    }

    usleep(500); // avoid busy waiting
}

destroyQueueHPF(readyQueue);
printf("HPF Scheduling finished.\n");
}

void RR_algo()
{
    int Quantum = 1;
    int processesCount = 0;
    struct CircularQueue myQ;
    initQueue(&myQ);

    bool success = 0;
    // PCB newProcess = RecieveProcess(success);
    while (true)
    {
        struct msgbuff myMsg = RecieveProcess(&success);
        if (success)
        {
            printf("\n recieved process with id: %d,arrival time : %d, at clock: %d\n", myMsg.data.processID, myMsg.data.arrivalTime, getClk());
            enqueueCirc(&myQ, myMsg.data);
            processesCount++;
            rotate(&myQ);
        }
        if (processesCount > 0)
        {

            // printQueue(&myQ);
            currentProcess = peekCurrent(&myQ);

            if (currentProcess->isFirstRun)
            {
                currentProcess->startTime = getClk();
                currentProcess->isFirstRun = false;

                char remaining_str[10];
                sprintf(remaining_str, "%d", currentProcess->remainingTime);

                currentProcess->processPID = fork();
                if (currentProcess->processPID == 0)
                {
                    execl("./process.out", "process", remaining_str, NULL);
                    perror("execl failed: check file name");
                    exit(-1);
                }
            }
            else
            {
                kill(currentProcess->processPID, SIGCONT);
            }
            int startQuantum = getClk();
            int currentTime;
            do
            {
                currentTime = getClk();
            } while (currentTime - startQuantum < Quantum);

            currentProcess->remainingTime -= Quantum;
            // printf("Current PID=%d, Remaining: %d, Id=%d\n\n", currentProcess->processID, currentProcess->remainingTime, currentProcess->processPID);
            if (currentProcess->remainingTime <= 0)
            {

                kill(currentProcess->processPID, SIGCONT);

                waitpid(currentProcess->processPID, NULL, 0);

                currentProcess->finishTime = getClk();
                printf("process #%d started at %d and finished at %d\n",
                       currentProcess->processPID,
                       currentProcess->startTime,
                       currentProcess->finishTime);
                removeCurrent(&myQ);
                processesCount--;
            }
            else
            {
                printf("\n%d\n", currentProcess->processPID);
                kill(currentProcess->processPID, SIGSTOP); // i want to verify this
                int status;
                pid_t result;
                do
                {
                    result = waitpid(currentProcess->processPID, &status, WUNTRACED | WNOHANG);
                } while (result == 0);

                if (result == currentProcess->processPID && WIFSTOPPED(status))
                {
                    printf("Child %d was stopped by signal %d\n", result, WSTOPSIG(status));
                }
                rotate(&myQ);
            }
            if (/* generatorDone!! && */ processesCount == 0)
            {
                break;
            }
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

    algorithm = atoi(argv[1]);
    while (true)
    {

        printf("\n recieved the algorithm: %d", algorithm);

        switch (algorithm)
        {
        case HPF:
            printf("\n processing with HPF...");
            HPF_algo();
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

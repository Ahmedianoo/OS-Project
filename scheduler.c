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
#include "SRTNQueue.h"




PCB runningProcess;

void processFinished_handler(int signum){ 


    int stat_loc;
    
    int pid = wait(&stat_loc);
    if (pid == -1) {
        perror("Error while waiting for a child process"); 
    } else if (WIFEXITED(stat_loc)) {
        int index = WEXITSTATUS(stat_loc);
        runningProcess.finishTime = getClk();
        printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
        //killpg(getpgrp(), SIGKILL);
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


msgbuff RecieveProcess(bool* success)
{
    struct msgbuff myMsg;
    int r = msgrcv(fromPGenQId, &myMsg, sizeof(myMsg) - sizeof(long), getpid() % 10000, IPC_NOWAIT);
    
    if (r != -1)
    {
        *success = true;
        return myMsg;
    }
    else
    {
        *success = false;
        struct msgbuff empty = {0};
        return empty;
    }
}





void SRTN_algo(){
    char remaining_str[10];
    struct msgbuff myMsg;
    PCB recProcess;
    runningProcess;
    bool success = 0;
    PriorityQueueSRTN* readyQueue = createQueue();

    while(success == 0  || myMsg.data.arrivalTime > getClk()){

        myMsg = RecieveProcess(&success);
    }

    runningProcess = myMsg.data;
    printf("\n recieved process with id: %d\n", runningProcess.processID);
    

    bool first = 1;
 
    int pStart;


    while(true){
        
        if(first){
            first = 0;
            if(runningProcess.remainingTime > 0){

                sprintf(remaining_str, "%d", runningProcess.remainingTime);
                runningProcess.processPID = fork();
                runningProcess.startTime = getClk();
                pStart = runningProcess.startTime;

                if (runningProcess.processPID == 0) {
                    execl("./process.out", "process", remaining_str,NULL);
                    perror("execl failed"); 
                    exit(1);
                }
            }

        }else { 
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
    
    
                        if (runningProcess.processPID == 0) {
                            execl("./process.out", "process", remaining_str,NULL);
                            perror("execl failed"); 
                            exit(1);
                        }
                        
                    }else{
                        kill(runningProcess.processPID, SIGCONT);
                        pStart = getClk();
    
                    }
    
                }


            }


        }

    }
    return;
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
            //call your function here
        break;

        case SRTN:
        printf("\n processing with SRTN...");
        SRTN_algo();
            
        break; 

        case RR:
        printf("\n processing with RR...");
          //call your function here  
        break;   
        
        default:
            break;
        }

    }
    // TODO implement the scheduler :)
    // Round Robin

    // upon termination release the clock resources
}

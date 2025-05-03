#include "headers.h"
#include "SRTNQueue.h"




PCB runningProcess;

bool contSRTN = false;
int noOfprocesses;
int noOfRec;
void processFinished_handler(int signum){ 


    int stat_loc;
    
    int pid = wait(&stat_loc);
    if (pid == -1) {
        perror("Error while waiting for a child process"); 
    } else if (WIFEXITED(stat_loc)) {
        int finish = WEXITSTATUS(stat_loc);
       runningProcess.finishTime = finish;
       //runningProcess.finishTime = getClk();
        runningProcess.remainingTime = 0;
        noOfprocesses--;
        noOfRec--;
        printf("process #%d has started at time %d and finished at %d.\n", pid, runningProcess.startTime, runningProcess.finishTime);
        contSRTN = true;
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
    runningProcess.startTime = getClk();
    runningProcess.pStart = runningProcess.startTime;
    printf("\n recieved process with id: %d\n", runningProcess.processID);
    noOfRec++;

    bool first = 1;
 



    while(true){
        
        if(first){
            first = 0;
            if(runningProcess.remainingTime > 0){


                //printf("%d\n", runningProcess.processPID);
                sprintf(remaining_str, "%d", runningProcess.remainingTime);


                runningProcess.processPID = fork();
                if (runningProcess.processPID == -1) {
                    perror("forking failed"); 
                    exit(1);
                }
                

                if (runningProcess.processPID == 0) {
                    execl("./process.out", "process", remaining_str,NULL);
                    perror("execl failed"); 
                    exit(1);
                } else {
                    
                    runningProcess.forked = 1;
                }
            }

        }else { 
            myMsg = RecieveProcess(&success);
            if(success) {noOfRec++;
                printf("number of recieved and not finished %d\n", noOfRec);
            }
            // printf("%d\n", noOfRec);
            // if(noOfRec == 0){
            //     printf("i am zero now no of processes");
            // }
            //count of received processes and check for them may you 
            //&& noOfRec > 0
            // if(noOfRec == 0){

            // }

            if(noOfprocesses > 0 && noOfRec == 0){
                continue;
            }


            if(noOfprocesses == 0){
                printf("program is finished bye!");
                break;

            }else if((contSRTN || (success && myMsg.data.arrivalTime <= getClk())) ){//there is an issue with the condition as we use the logic built on success
                contSRTN = false;


               //printf("i am here in the srtn\n");
                if(success){
                    printf("i have succeed\n");
                    recProcess = myMsg.data;
                    enqueue(readyQueue, recProcess);

                }
               
                //printf("i am here after the enqueue with id %d\n", readyQueue->head->data.remainingTime);
                if(runningProcess.remainingTime == 0 || readyQueue->head->data.remainingTime < runningProcess.remainingTime){
                   //printf("i am here after the enqueue with id %d, i have remaining time %d, the pstart\n", recProcess.processID, recProcess.remainingTime);
                    int tempclk = getClk();
                    

                    //printf("i am the running process %d\n", runningProcess.processID);
                    if(runningProcess.finishTime ==  -1){
                        //printf("remaining time %d %d\n", runningProcess.processPID, runningProcess.remainingTime);
                        runningProcess.remainingTime = runningProcess.remainingTime - (tempclk - runningProcess.pStart);
                        //printf("remaining time %d %d\n", runningProcess.processPID, runningProcess.remainingTime);
                        runningProcess.pStart = tempclk;
                        runningProcess.last_scheduled_time = tempclk;
                        //printf("before sigstop, i am the running process %d\n", runningProcess.processPID);
                        enqueue(readyQueue, runningProcess);
                        kill(runningProcess.processPID, SIGSTOP);
                        runningProcess = dequeue(readyQueue);

                    }else {

                        //printf("i am the running process %d\n", runningProcess.processID);
                        runningProcess = dequeue(readyQueue);
                        //printf("i am the running process %d\n", runningProcess.processPID);
                        //printf("i am the running process %d\n", runningProcess.processID);

                    }
                    //printf("i am the running process %d\n", runningProcess.processID);
                    //printf("%d\n", runningProcess.processPID);
                    
    
                    if(runningProcess.forked == -1){// check if there is a better way to do this check
    
    
                        printf("\n recieved process with id: %d\n", runningProcess.processID);
 

                        runningProcess.startTime = getClk();
                        runningProcess.pStart = runningProcess.startTime;
                        sprintf(remaining_str, "%d", runningProcess.remainingTime);
                        runningProcess.processPID = fork();
                        if (runningProcess.processPID == -1) {
                            perror("forking failed"); 
                            exit(1);
                        }
                        
    
                        if (runningProcess.processPID == 0) {
                            execl("./process.out", "process", remaining_str,NULL);
                            perror("execl failed"); 
                            exit(1);
                        }else {
                    
                            runningProcess.forked = 1;
                        }
                        
                    }else{
                        printf("i will cont. now: %d\n", runningProcess.processPID);
                        contSRTN = false;
                        kill(runningProcess.processPID, SIGCONT);
                        runningProcess.pStart = getClk();
    
                    }
    
                }


            }


        }

        //usleep(1000);

    }

    //waitclk();
    return;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\nhello\n");
    initClk();
    InitComGentoScheduler();
    signal(SIGUSR1, processFinished_handler);
    noOfprocesses = atoi(argv[2]);
    
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

#include "headers.h"
#include "helpers.h"

#define MAXPROCESSES 100

int toSchedulerQId;
pid_t schedulerID;

void clearResources(int signum)
{

    destroyClk(true);
    printf("clock destroyed  successfully");
    destroyClk(true);
    printf("clock destroyed  successfully");
    //  TODO Clears all resources in case of interruption
    if (msgctl(toSchedulerQId, IPC_RMID, NULL) == -1)
    {
        perror("msgctl - IPC_RMID");
    }
    else
    {
        printf("Scheduler Message queue removed successfully.\n");
    }
    exit(1);
}

void InitComGentoScheduler()
{
    toSchedulerQId = msgget(ProcessGenToSchedulerQKey, 0666 | IPC_CREAT);
    if (toSchedulerQId == -1)
    {
        perror("process gen to scheduler msg Q");
    }
}

void SendToScheduler(PCB dataToSend)
{
    struct msgbuff msg;
    msg.mtype = schedulerID % 10000; // can hardcode the type to the type of the scheduler but for later
    msg.data = dataToSend;
    if (msgsnd(toSchedulerQId, &msg, sizeof(msg.data), !IPC_NOWAIT) == -1)
    {
        perror("msgsnd failed");
    }
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    signal(SIGINT, clearResources);
    InitComGentoScheduler();

    // TODO Initialization
    // 1. Read the input files.
    PCB processes[MAXPROCESSES];
    FILE *file = fopen("processes.txt", "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    char line[100];
    fgets(line, sizeof(line), file); // skipping the first line in the text file

    int noOfProcesses = 0;
    while (fscanf(file, "%d  %d  %d  %d",
                  &processes[noOfProcesses].processID,
                  &processes[noOfProcesses].arrivalTime,
                  &processes[noOfProcesses].runtime,
                  &processes[noOfProcesses].processPriority) == 4)
    {
        processes[noOfProcesses].remainingTime = processes[noOfProcesses].runtime;
        processes[noOfProcesses].isFirstRun = true;
        noOfProcesses++;
    }

    for (int i = 0; i < noOfProcesses; i++)
    {
        printf("%d %d\n", processes[i].processID, processes[i].arrivalTime); // this will be commented later
    } // it is just for printing the list of processes

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Select the scheduling algorithm: \n1-HPF  \n2-SRTN  \n3-RR\n");
    enum algorithms algorithm;
    // scanf("%d", &algorithm);

    int alg_choice;
    scanf("%d", &alg_choice);
    algorithm = (enum algorithms)alg_choice;

    if (algorithm < 1 || algorithm > 3)
    {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }
    int RRQuantum = 1;
    char quantum_str[12]; // Buffer to hold the string representation

    switch (algorithm)
    {
    case RR:
        printf("You selected Round Robin (RR)\n");
        while (true)
        {
            printf("Insert the Quantum needed: ");
            scanf("%d", &RRQuantum);
            if (RRQuantum > 0)
            {
                break;
            }
            else
            {
                printf("Invalid Qunatum!\n");
            }
        }
        sprintf(quantum_str, "%d", RRQuantum);
        break;
    case SRTN:
        printf("You selected Shortest Remaining Time Next (SRTN)\n");
        break;
    case HPF:
        printf("You selected Highest Priority First (HPF)\n");
        break;
    }

    // 3. Initiate and create the scheduler and clock processes.

    schedulerID = fork();
    if (schedulerID == 0)
    {
        char no[10]; 
        sprintf(no, "%d", noOfProcesses);
        printf("I am the schedular with PID: %d\n", getpid());
        execl("./scheduler.out", "scheduler", algorithmToString(algorithm), no, quantum_str,"I am the schedular, the process manager has just created me", NULL);
        perror("execl failed");
        return 0;
    }
    else if (schedulerID == -1)
    {
        printf("\nError in forking the schedular. PID: %d\n", getpid());
        perror("fork failed\n");
        exit(1);
    }

    pid_t clockID = fork();
    if (clockID == 0)
    {
        printf("I am the clock with PID: %d\n", getpid());
        execl("./clk.out", "clk", "I am the clock, the process manager has just created me", NULL);
        perror("execl failed: check file name");
        exit(-1);
    }
    else if (clockID == -1)
    {
        printf("\nError in forking the clockID. PID: %d\n", getpid());
        perror("fork failed\n");
        exit(1);
    }

    if(algorithm == SRTN){
        qsort(processes, noOfProcesses, sizeof(PCB), comparePCBForSRTN);
    }

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x, sent = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (sent < noOfProcesses)
    {
        x = getClk();
        // something should be done when clk or arrival time = 0
        while (sent < noOfProcesses && processes[sent].arrivalTime <= x)

        {
            //printf("\nclock at send %d\n", x);
 
            processes[sent].finishTime = -1;
            processes[sent].finished = 0;
            processes[sent].forked = -1;
            processes[sent].processPPID = getpid();
            SendToScheduler(processes[sent]);
            sent++;
        }

        // sleep(1);  // or a shorter delay if needed
    }

    while (true)
    {
        //printf("process gen is done\n");
        sleep(1);
    }
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    // wait(&status);
}

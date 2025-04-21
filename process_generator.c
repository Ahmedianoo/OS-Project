#include "headers.h"

#define MAXPROCESSES 100

void clearResources(int);
int toSchedulerQId;
void InitComGentoScheduler()
{
    toSchedulerQId = msgget(schedulerQKey, 0666 | IPC_CREAT);
    if (toSchedulerQId == -1)
    {
        perror("process gen to scheduler msg Q");
    }
}
void SendToScheduler(PCB dataToSend, long mtype)
{
    struct msgbuff msg;
    msg.mtype = mtype; // can hardcode the type to the type of the scheduler but for later
    msg.data = dataToSend;
    msgsnd(toSchedulerQId, &msg, sizeof(msg.data), !IPC_NOWAIT);
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
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

    if (algorithm < 0 || algorithm > 2)
    {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    switch (algorithm)
    {
    case RR:
        printf("You selected Round Robin (RR)\n");
        break;
    case SRTN:
        printf("You selected Shortest Remaining Time Next (SRTN)\n");
        break;
    case HPF:
        printf("You selected Highest Priority First (HPF)\n");
        break;
    }

    // 3. Initiate and create the scheduler and clock processes.

    pid_t schedulerID = fork();
    if (schedulerID == 0)
    {
        printf("I am the schedular with PID: %d\n", getpid());
        execl("scheduler", "scheduler", "I am the schedular, the process manager has just created me", NULL);
        return 0;
    }
    else if (schedulerID == -1)
    {
        printf("\nError in forking the schedular. PID: %d\n", getpid());
        perror("fork failed");
        exit(1);
    }

    pid_t clockID = fork();
    if (clockID == 0)
    {
        printf("I am the clock with PID: %d\n", getpid());
        execl("clk", "clk", "I am the clock, the process manager has just created me", NULL);
        perror("execl failed");
        exit(-1);
    }
    else if (clockID == -1)
    {
        printf("\nError in forking the clockID. PID: %d\n", getpid());
        perror("fork failed");
        exit(1);
    }

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this

    while (true)
    {
        int x = getClk();
        printf("current time is %d\n", x);
    }
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    destroyClk(true);
    // TODO Clears all resources in case of interruption
    if (msgctl(toSchedulerQId, IPC_RMID, NULL) == -1)
    {
        perror("msgctl - IPC_RMID");
    }
    else
    {
        printf("Message Up queue removed successfully.\n");
    }
    exit(1);
}

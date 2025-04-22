#include "headers.h"

int fromPGenQId;
void InitComGentoScheduler()
{
    fromPGenQId = msgget(ProcessGenToSchedulerQKey, 0666 | IPC_CREAT);
    if (fromPGenQId == -1)
    {
        perror("process gen to scheduler msg Q");
    }
}
msgbuff RecieveProcess()
{
    struct msgbuff myMsg;
    msgrcv(fromPGenQId, &myMsg, sizeof(myMsg.data), getpid() % 10000, !IPC_NOWAIT);
    return myMsg;
}

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\nhello\n");
    initClk();
    InitComGentoScheduler();

    struct msgbuff myMsg;
    while (true)
    {

        myMsg = RecieveProcess();
        printf("\n recieved process with id: %d\n", myMsg.data.processID);
    }
    // TODO implement the scheduler :)
    // upon termination release the clock resources
}

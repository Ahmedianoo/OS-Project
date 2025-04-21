#include "headers.h"

int fromPGenQId;
void InitComGentoScheduler()
{
    fromPGenQId = msgget(schedulerQKey, 0666 | IPC_CREAT);
    if (fromPGenQId == -1)
    {
        perror("process gen to scheduler msg Q");

    }
}
int main(int argc, char *argv[])
{
    initClk();
    InitComGentoScheduler();
    // TODO implement the scheduler :)
    // upon termination release the clock resources

    destroyClk(true);
}

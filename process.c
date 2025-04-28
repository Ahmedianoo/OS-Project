#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

// void sigusr2handler()
// {
//     printf("got signaled i am %d", getpid());
// }

int main(int agrc, char *argv[])
{
    // setvbuf(stdout, NULL, _IONBF, 0);
    // signal(SIGUSR2, sigusr2handler);
    initClk();

    if (agrc < 2)
    {
        fprintf(stderr, "Missing argument for remaining time\n");
        exit(1);
    }

    // TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    while (remainingtime > 0)
    {
        // printf("Remaining Time from process %d: %d\n", getpid(), remainingtime);
        printf("I process %d: will take from clock %d to %d\n", getpid(), getClk(), getClk() + 1);
        waitclk(); // this function is defined in the header file and it waits for the clock tik
        remainingtime--;
    }
    // signal the scheduler to tell him you finished
    kill(getppid(), SIGUSR1);
    destroyClk(false);

    return 0;
}

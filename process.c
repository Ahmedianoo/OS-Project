#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

// void sigusr2handler()
// {
//     raise(SIGSTOP);
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

        printf("Remaining Time from process %d: %d at %d\n", getpid(), remainingtime, getClk());
        // printf("I process %d: will take from clock %d to %d\n", getpid(), getClk(), getClk() + 1);

        remainingtime--;
        waitclk(); // this function is defined in the header file and it waits for the clock tik
        // if (remainingtime > 0)
        // {
        //     raise(SIGSTOP);
        // }
    }
    // signal the scheduler to tell him you finished
    kill(getppid(), SIGUSR1);
    // destroyClk(false);
    exit(getClk());

    return 0;
}

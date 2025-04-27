#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();
    
    if (agrc < 2) {
        fprintf(stderr, "Missing argument for remaining time\n");
        exit(1);
    }
    
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    while (remainingtime > 0)
    {
        printf("Remaining Time from process %d: %d\n",getpid(),remainingtime);
        waitclk();      //this function is defined in the header file and it waits for the clock tik
        remainingtime--;
    }
    //signal the scheduler to tell him you finished
    kill(getppid(), SIGUSR1);
    destroyClk(false);
    
    return 0;
}

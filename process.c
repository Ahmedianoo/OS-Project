    #include "headers.h"

    /* Modify this file as needed*/
    int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();

    int temp;

    if (agrc < 2) {
        fprintf(stderr, "Missing argument for remaining time\n");
        exit(1);
    }
    
    //TODO it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    temp = remainingtime;
    int finish;
    while (remainingtime > 0)
    {
        //printf("i am the process %d: %d\n", getpid(), remainingtime);
        waitclk();      //this function is defined in the header file and it waits for the clock tik
        remainingtime--;
        
    }

    finish = getClk();
    //signal the scheduler to tell him you finished

    //printf("the right child is called with id %d and runtime %d\n", getpid(), temp);
    kill(getppid(), SIGUSR1);
    exit(finish % 256);
    destroyClk(false);
    
    
}

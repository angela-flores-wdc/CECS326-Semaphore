// created by Angela Flores
// CECS 326 Semaphore Lab

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "semaphore.h"

using namespace std;

//set the number of child processes we will be creating
const int processNum = 10;

// names of my four semaphores (one semaphore for every float that will be modified and MUTEX to handle output)
enum {SEM1, SEM2, SEM3, SEM4, MUTEX}; 


int main(){
 
    // initalize all necessary variables
    long childPid = 1 ;
    long wpid;
    int status = 0;
    int shmid;
    key_t key = ftok("shmfile", 65);

    //initalize my five semaphores 
    SEMAPHORE sem(5);

    //increment each semaphore to 1 so they are ready to be used by the child processes
    sem.V(SEM1);
    sem.V(SEM2);
    sem.V(SEM3);
    sem.V(SEM4);
    sem.V(MUTEX);

    //initalize shared memory
    shmid = shmget(key, sizeof(float), IPC_CREAT| 0666);
    if( shmid == -1){
        perror("shmget failed: ");
        exit(1);
    }

    //initialize void pointer
    void *sharedFloats = (void*)0;

    //point pointer to shared memory and cast to float
    sharedFloats = (float*)shmat(shmid, NULL, 0);

    //initialize float pointer and set it to shared memory
    static float *shTest;
    shTest = (float*)sharedFloats;

    //set elements of float array to be modified later
    shTest[0] = 2.0;
    shTest[1] = 2.5;
    shTest[2] = 1.5;
    shTest[3] = 5.5;


    //for loop to fork child processes
    for (int i = 0; i < processNum; i++){

        //create child process with fork() function
        childPid = fork();
        
        //if we are in the child process
        if (childPid == 0){         

            //in child process, iterate a set number of times, in this case 3 times
            for (int x =0; x<3;x++){

                //seed random number generator
                srand(time(0)^getpid());

                //generate random integer between 0 and 3 (so we can modify one of the four floats in shTest)
                int rando = (rand() % 4);
                
                //based on which float we will be modifiying, call P to decrement the semaphore associated with that
                //float so any other process attempting to modify that float will be blocked
                switch(rando){
                    case 0: sem.P(SEM1);
                            break;
                    case 1: sem.P(SEM2);
                            break;
                    case 2: sem.P(SEM3);
                            break;
                    case 3: sem.P(SEM4);
                            break;
                }

                //call P function on MUTEX semaphore so any other process trying to modify a float or output to 
                //console will be blocked
                sem.P(MUTEX);
                //beginning of critical section
                cout << "Process " << getpid() << " is operating on mem " << rando << endl;
                cout << "starting value: " << shTest[rando] << endl;
                //perform modification on float
                float beta = -0.5 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(0.5-(-0.5))));
                cout << "beta: " << beta << endl;
                shTest[rando] = shTest[rando] + beta;
                cout<<"new value: " << shTest[rando] << endl;
                cout << endl;
                //end of critical section
                //call V function on MUTEX to allow other process to enter into critical section
                sem.V(MUTEX);  
                
                //call V function on semaphore associated to the float we just modified so another process 
                //may now modify it
                switch(rando){
                    case 0: sem.V(SEM1);
                            break;
                    case 1: sem.V(SEM2);
                            break;
                    case 2: sem.V(SEM3);
                            break;
                    case 3: sem.V(SEM4);
                            break;
                }
            }
            //exit the child program
            exit(0);
        }
        //if the process creation has failed
        if (childPid < 0){
            cout << "Error creating child process" << endl;
            //exit program with error status
            exit(1);
        }
    }

    //while loop to make aprent halt execution until ALL child processes have terminated
    while ((wpid = wait(&status)) > 0); 

    //parent clean-up
    cout << "Parent cleaning up..." << endl;

    //removing shared memory
	shmctl(shmid, IPC_RMID, NULL);	

    //remove semaphores
	sem.remove();
    cout << "All done!" << endl;

    //exit parent process
    exit(0);
}
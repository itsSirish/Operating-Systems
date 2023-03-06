//GROUP 10

//Group Members

//Visweswar Sirish Parupudi 2020AAPS0330H
//Konkala Rithvik 2020A8PS0517H
//Shreya Senapaty 2020AAPS0309H
 
//Kshitij Agarwal 2020A3PS2209H
//Rishabh Khandelwal 2020A8PS2154H
//Raj Jagtap 2020AAPS2113H
//Jinil Bhavin Shah 2020AAPS1750H
//Suraj Jayakumar 2020AAPS2105H

//compile the code using : gcc -o burger burger.c -lpthread
//execute test case  using: ./burger 4 10 99 5 17 28 67 65 22 19 11 77 89 78 45 40 20 10 90 76
//this code was written on an ubuntu 18.04 system

//LIBRARIES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h> //these are all the required libraries

//VARIABLES
int n; //global variable n for matrix dimensons
int p; //global variable p for no. of primes
int x; //global variable x for element in matrix
int a; //global variable a for lower bound of range
int b; //global variable b for lower bound of range
int avg_thapx; // global variable for thapx value for each x
int err; //global variable flag to detect error in validation of range
int es; //global variable to detect exit status of child process

//FUNCTIONS
int inRange(int x,int a,int b){ //function to check if x lies between a and b
    if(x<a || x>b)
    return 1; //returns 1 for false/error
    else
    return 0;
}

int isPrime(int n){// standard function to check if a number is prime or not

    int i, flag = 0;
    if (n == 0 || n == 1)
        flag = 1;

    for (i = 2; i <= n / 2; ++i) {
        if (n % i == 0) {
            flag = 1;
            break;
        }
    }
     // flag is 0 for prime numbers
    if (flag == 0)
        return 1;
    else
        return 0;
}

int calculateAverage(int arr[],int n){ //function to calculate average of an array as input
    int average;
    int size = n;
    int sum=0;
    for(int i=0;i<n;i++){
        sum = sum+arr[i];
    }
    average = sum/n;
    return average;
}

//FUNC EXECUTED BY EACH THREAD
void* calculatePAverage(){ //function to calculate the thapx value for every x
    //this function is executed by a thread as seen later

    err=inRange(x,a,b);//checks if x satisfies a<=x<=b
    if (err==1){
        printf("error detected!! %d is out of bounds\n",x);
        pthread_exit(NULL);//tread terminates and returns control to worker process
    }
    printf("Worker thread is executing\n");
    int average;
    int sum=0;
    int count = 0;

    int num = x - 1;
    //Generate p prime nums before x
    printf("Prime numbers before %d: ", x);
    while (count < p&&num>1) {
        if (isPrime(num)) {
            printf("%d ", num);
            count++;
            sum = sum+num; 
        }
        num--;
    }
    int c1 = count;
 
    // Generate p prime nums after x
    printf("\nPrime numbers after %d: ", x);
    count = 0;
    num = x + 1;
    while (count < p) {
        if (isPrime(num)) {
            printf("%d ", num);
            count++;
            sum = sum+num;
        }
        num++;
    }
    
    average = sum/(count+c1); 

    avg_thapx = average; //we store the average in thapx value for every x in matrix
    printf("\nThis thread is terminated");

    pthread_exit(NULL); //the thread is terminated

}

//FUNC FOR SIGCHLD HANDLER
void proc_exit(int sig){

    int chck; 
    pid_t pid;
    printf("Entering handler\n");
    // while(1){
        //we check for exit status of the child to see if it terminated without error or not
    //     pid = wait3(&chck, WNOHANG, (struct rusage *)NULL );

    //     if(chck==0){
    //         printf("Child procees has been terminated\n");
    //         return;
    //     }
    //     else{
    //         printf("we are in else\n\n\n");
    //         fprintf(stderr,"Process with PID : %d exited with return value : %d\nTerminating all processes!", pid, chck);
    //         kill(0,9);
    //     }
    // }
}

//MAIN FUNCTION
int main (int argc, char* argv[]){ //main fucntion which takes inputs through command line
    
    n = atoi(argv[1]); //n for dimensions of matrix
    a = atoi(argv[2]); //lower bound
    b = atoi(argv[3]); //upper bound
    p = atoi(argv[4]); //no. of primes
    int arr[n][n]; //initialising 2d matrix
    int j = 5; //from input 5 onwards we start filling in the matrix
    for(int i=0;i<n;i++){
        for(int k =0;k<n;k++)
        {
            arr[i][k] = atoi(argv[j]);//we fill up the matrix
            j++; 
        }  
    }

    printf("The recieved matrix is\n");
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            printf("%d ",arr[i][j]); //prinitng the input matrix
        }
        printf("\n");
    }

    int wpapx[n]; //this is our array of wpapx values for n rows which the parent process will use to calculate fapx
    pid_t parent;
    
    parent = getpid(); //pid of parent
    
    for(int i=0;i<n;i++)//we start a for loop to creat n child processes
    {   
        //SIGCHLD signal which activates every time a child terminates and is called by the parent process
        signal(SIGCHLD,proc_exit);
        int fd[2]; //this is for pipelining, fd[0] for read and fd[1] for write
        if(pipe(fd)){
            printf("Error occ");
        }
        int pid;
        pid = fork();//we fork the main process , returns 0 for child and pid of child to parent

        //CHILD
        if(pid==0) //this is for child process
        {   
            printf("*******************************\n");
            printf("This is worker process number %d\n",i+1);
            printf("[worker] pid %d from [parent] pid %d\n",getpid(),getppid());
            printf("Processing row number %d\n",i+1);

            close(fd[0]);//we close the read pipe
            int thapx[n];//this is the array for thapx values of one row

            pthread_t tid;//we initilaise our thread

            for(int j=0;j<n;j++){//we creat n threads for each child process

                x = arr[i][j];//store each element in a variable x
                
                printf("\nThis is thread number %d :: spawned by worker process %d\n",j+1,i+1);

                //THREADS
                // pthread_t th[n];// array of n threads could also be used

                if (pthread_create(&tid,NULL,&calculatePAverage,NULL)!=0){//thread executes calculatePAverage to succesfully calculate thapx
                    perror("failed to create thread\n");
                }

                if (pthread_join(tid,NULL)!=0){//joins threads
                }

                if(avg_thapx==-1){ //if our avg_thapx value isnt updated that means the thread executed with error
                    printf("Exiting from Child Process\n");
                    exit(1);
                }

                thapx[j] = avg_thapx;//we update the array with values of thapx
                printf("\nThe thapx value of %d is %d\n",arr[i][j],avg_thapx);
                avg_thapx=-1; //we reset our thapx every time
                // pthread_exit(NULL);
                

            }
 
            printf("\nThe thapx values from row %d are : ",i+1);//we print the value of thapx values for one row

            for(int k=0;k<n;k++)
            {
                printf("%d ",thapx[k]);

            }
            printf("\n");
            //now we calculate wpapx for every row
            int avg_wpapx = calculateAverage(thapx,n);
            printf("Calculating wpapx\n");
            printf("wpapx = %d\n",avg_wpapx);
            printf("writing wpapx onto the controller\n");
            //the child process write the value of wpapx to the pipe to be read by the parent
            write(fd[1],&avg_wpapx,sizeof(int));
            close(fd[1]);
            
            printf("Child terminated without error\n");
            exit(0);//exits succesfully 
        }


        
        //PARENT
        if(getpid()==parent){
            //this is for parent process
                int chck; 
                //we check for exit status of the child to see if it terminated without error or not
                if ( waitpid(pid, &chck, 0) == -1 ) {
                    perror("waitpid() failed");
                }
                if ( WIFEXITED(chck) ) {
                    es = WEXITSTATUS(chck);
                    printf("Exit status was %d\n", es);
                    if(es!=0){
                        printf("Parent kills everything\n");
                        printf("All children were terminated\n");
                        printf("Please Try Again with valid input!\n");
                        printf("*******************************\n");
                        kill(0,9);
                    }
                }
                close(fd[1]);//write pipe is closed
                int y;
                read(fd[0],&y,sizeof(int));//we read from the pipeline
                wpapx[i]=y;//y is read from pipe and updated to wpapx array
                printf("Controller has recieved wpapx = %d from worker process %d through the pipe\n",y,pid);//pid is for child
                close(fd[0]);//read pipe is closed
                printf("End of process %d\n",i+1);
                printf("\n");
        }

    }
    printf("*******************************\n");

    printf("\nThe wpapx values captured so far are : ");

            for(int k=0;k<n;k++)
            {
                printf("%d ",wpapx[k]);
            }

    printf("\n");        

    int fapx = calculateAverage(wpapx,n);//finally fapx value is calculated
    printf("The value of fapx is: %d\n",fapx);

}

//******end of code*************




//we have also written an alternative to this code using n*n pipelines instead of n pipelines and n threads
//this gives the output as required as well

// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/wait.h>

// //check if a no is prime or not 
// int isPrime(int n){

//     int i, flag = 0;
//     if (n == 0 || n == 1)
//         flag = 1;

//     for (i = 2; i <= n / 2; ++i) {
//         if (n % i == 0) {
//             flag = 1;
//             break;
//         }
//     }
//      // flag is 0 for prime numbers
//     if (flag == 0)
//         return 1;
//     else
//         return 0;
// }

// //calculates the average of a given array here arr and size are inputs
// int calculateAverage(int arr[],int n){
//     int average;
//     int size = n;
//     int sum=0;
//     for(int i=0;i<n;i++){
//         sum = sum+arr[i];
//     }
//     average = sum/n;
//     //printf("The sizeof the array is %d\n",size);
//     //printf("The avg of the array is %d\n",average);
//     return average;
// }


// //calculates the average of a select set of numbers in an array
// int calculatePAverage(int x,int p){
//     int average;
//     int sum=0;
//     int count = 0;
//     int num = x - 1;
//     printf("Prime numbers before %d: ", x);
//     while (count < p&&num>1) {
//         if (isPrime(num)) {
//             printf("%d ", num);
//             count++;
//             sum = sum+num; 
//         }
//         num--;
//     }
//     int c1 = count;
 
//     // Generate p prime numbers after x
//     printf("\nPrime numbers after %d: ", x);
//     count = 0;
//     num = x + 1;
//     while (count < p) {
//         if (isPrime(num)) {
//             printf("%d ", num);
//             count++;
//             sum = sum+num;
//         }
//         num++;
//     }
    
//     average = sum/(count+c1);

//     return average;
// }


// int main (int argc, char* argv[])
// {
//     int n,p;

//     printf("Enter the number n: ");
//     scanf("%d",&n);
//     int arr[n][n];
//         for(int i=0;i<n;i++){    
//         for(int j=0;j<n;j++){    
//             printf("Enter the number n[%d][%d]: ",i,j);
//             scanf("%d",&arr[i][j]);    
//             }    
//         }
  
    
//     printf("Enter the number p: ");
//     scanf("%d",&p);
//     int count=0;
//     int j =0;
    
//     int PAverages[n];

//     pid_t parent;
//     parent = getpid();

//     for(int i=0;i<n;i++) // loop will run n times (n=5)
//     {
//         int fd[2];
//         if(pipe(fd)){
//             printf("Error occ");
//         }
        
        

//         if(fork() == 0)//beginning of layer 1
//         {
//             printf("************************\n");
//             printf("This is process number %d\n",i+1);
//             printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid());
//             printf("The children are:\n\n");
//             pid_t parent;
//             int thapx=0;
//             parent = getpid(); //link 1

//             for(int j=0;j<n;j++) // loop will run n times (n=5)
//             {
//                 int fd[2];
//                 if(pipe(fd)){
//                     printf("Error occ");
//                 }
                
                

//                 if(fork() == 0)// beginning of layer 2
//                 {
                    
                    
//                     close(fd[0]);
//                     int x = calculatePAverage(arr[i][j],p);
//                     printf("\nthapx = %d\n",x);
//                     write(fd[1],&x,sizeof(int));
//                     close(fd[1]);
//                     exit(0);

//                 }
//                 else if(getpid()==parent){
//                     for(int i=0;i<n;i++) // loop will run n times (n=5)
//                     {
//                         wait(NULL);
                    

//                     }
//                     close(fd[1]);
//                     int y;
//                     read(fd[0],&y,sizeof(int));
//                     PAverages[j]=y;
//                     close(fd[0]);
                
//                     //printf("Got from child process %d\n",y);
//                 }
//                 if(getpid()!=parent){
//                     for(int i=0;i<n;i++) // loop will run n times (n=5)
//                     {
//                         wait(NULL);
                    

//                     }
                    

//             }
//             }

//             for(int i=0;i<n;i++)//read from layer 2
//             {
//                 //printf("Got this element from child process %d\n",PAverages[i]);
//                 thapx = thapx+PAverages[i];
//             }

//             int wpapx = calculateAverage(PAverages,n);
//             //printf("The value of wpapx is : %d\n",wpapx);

//                     close(fd[0]);//reading back to parent
//                     int x = wpapx;
//                     printf("\nparent reads wpapx as: %d\n",x);
//                     write(fd[1],&x,sizeof(int));
//                     close(fd[1]); 
                    
//                     exit(0);

//                 }
//                 else if(getpid()==parent){
//                     for(int i=0;i<n;i++) // loop will run n times (n=5)
//                     {
//                         wait(NULL);
                    

//                     }
//             close(fd[1]);//link 2
//             int y;
//             read(fd[0],&y,sizeof(int));
//             PAverages[i]=y;
//             close(fd[0]);
        
//             //printf("Got from child process %d\n",y);
//         }
//     if(getpid()!=parent){
//         for(int i=0;i<n;i++) // loop will run n times (n=5)
//         {
//             wait(NULL);
        

//         }
        

//     }
//     }

//     for(int i=0;i<n;i++)
//     {
//         //printf("Got this element from child process %d\n",PAverages[i]);
//         i++;
//     }
//     int fapx = calculateAverage(PAverages,n);
//     printf("\ncontroller reads fapx as: %d\n\n",fapx);
    

//     return 0;
// }
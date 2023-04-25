//  TEAM MEMBERS :
// SRIRAM KASHYAP
// SHIVANSH SHUKLA
// SRIRSH VISWESWAR
// MUSKAAN KUMAR
// RYA SANOVAR
// VAISHNAVI SHRESHTHI
// ANURAG GADE
// RITHVIK KONKALA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define CONNECT_CHANNEL_NAME "/connect_channel"
#define COMM_CHANNEL_NAME "/comm_channel"

typedef struct
{
    int key;
    int client_counter;
    int n_clients;
    int request_type;    //-1 to disconnect and 1 to connect
    int keys[100];

    union
    {
        struct
        {
            int n1, n2, op;
        } arithmetic;
        struct
        {
            int n1;
        } EvenorOdd;
        struct
        {
            int n1;
        } isPrime;
        struct
        {
            int n1;
        } isNegative;
    } request_payload;

    int is_data_ready; //flag for client writing

    union
    {
        struct
        {
            int ans;
        } arithmetic;
        struct
        {
            int val; // 0 for even
        } EvenorOdd;
        struct
        {
            int val; // 0 for prime
        } isPrime;
        struct
        {
            int val; // 0 for negative
        } isNegative;
    } response_payload;

    int is_response_ready; // flag for response from server

    pthread_mutex_t mutex;

    pthread_mutex_t res_mutex;

} client_request;

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
int request_count = 0;
void print_keys(int keys[1000], int num_clients, int response_sent)
{   printf("SUMMARY: ------- \n");
    printf("\nNumber of clients: %d\n", num_clients);
    printf("Keys: ");
    for (int i = 0; i < num_clients; i++)
    {
        printf("%d, ", keys[i]);
    }
    // request_count++;
    
    printf("\nResponse sent: %d\n", response_sent);
}

void *runner(void *key){
    
    int current_key = (int *)key;
    char comm_channel_name[] = "/comm_channel";
    char buffer[10];

    sprintf(buffer, "%d", current_key);

    strcat(comm_channel_name, buffer);

    int comm_channel_fd = shm_open(comm_channel_name, O_CREAT | O_RDWR, 0666);
    if (comm_channel_fd == -1)
    {
        perror("shm_open_comm_channel");
        exit(1);
    }

    if (ftruncate(comm_channel_fd, sizeof(client_request)) == -1)
    {
        perror("ftruncate_comm_channel");
        exit(1);
    }

    client_request *comm_channel = (client_request *)mmap(NULL, sizeof(client_request), PROT_READ | PROT_WRITE, MAP_SHARED, comm_channel_fd, 0);

    if (comm_channel == MAP_FAILED)
    {
        perror("mmap_comm_channel");
        exit(1);
    }
    

    int temp = 0;

    comm_channel->is_data_ready = 0;
    comm_channel->is_response_ready = 0;
    comm_channel->client_counter = 0;
    // comm_channel->n_clients = 0;
    comm_channel->key = current_key;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&comm_channel->mutex, &mutex_attr);

    pthread_mutexattr_t mutex_attr2;
    pthread_mutexattr_init(&mutex_attr2);
    pthread_mutexattr_setpshared(&mutex_attr2, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&comm_channel->res_mutex, &mutex_attr2);

    
  

    while(1){

        switch (comm_channel->request_type)
        {

        case 1: // arithemtic code
        {   printf("Comm Channel is created successfully!! \n");
            printf("Comm Channel for client with key : %d is up and running\n", comm_channel->key);

            while (comm_channel->is_data_ready == 0)
            {
            }
            // read
            int n1 = comm_channel->request_payload.arithmetic.n1;
            int n2 = comm_channel->request_payload.arithmetic.n2;
            int op = comm_channel->request_payload.arithmetic.op;

            comm_channel->is_data_ready = 0;

            // string oper;
            // if(op == 1){
            //     oper = "addition";
            // }
            // else if(op == 2){
            //     oper = "subtraction";
            // }
            // else if(op == 3){
            //     oper = "multiplication";
            // }
            // else if(op == 4){
            //     oper = "division";
            // }
            // else{
            //     printf("Invalid operation \n");
            // }
            printf("Received request from user with key : %d with num %d %d to perform %d \n",comm_channel->key, n1, n2, op);

            pthread_mutex_lock(&comm_channel->res_mutex);
            comm_channel->client_counter = ++temp;
            switch (op)
            {
            case 1: // operator = +
            {
                comm_channel->response_payload.arithmetic.ans = n1 + n2;
            }
            break;

            case 2: //operator = -
            {
                comm_channel->response_payload.arithmetic.ans = n1 - n2;
            }
            break;

            case 3: //operator = *
            {
                comm_channel->response_payload.arithmetic.ans = n1 * n2;
            }
            break;

            case 4: //operator = /
            {
                comm_channel->response_payload.arithmetic.ans = n1 / n2;
            }
            break;

            default:
                break;
            }
            comm_channel->request_type = 0;
            comm_channel->is_response_ready = 1;
            request_count++;
            printf("Server has responded to client!! \n");
            printf("Total Number of requests so far is %d\n",request_count);
            pthread_mutex_unlock(&comm_channel->res_mutex);
        }
        break;

        case 2: // evenorodd
        {   printf("Comm Channel is created successfully!! \n");
            printf("Comm Channel for client with key : %d is up and running\n", comm_channel->key);
            while (comm_channel->is_data_ready == 0)
            {
            }

            // read
            int n = comm_channel->request_payload.EvenorOdd.n1;
            comm_channel->is_data_ready = 0;

            printf("Recieved request from user with key : %d to check whether %d is even or odd \n",comm_channel->key, n);
            // comm_channel->request_type=0;

            pthread_mutex_lock(&comm_channel->res_mutex);
            // write
            comm_channel->request_type = 0;
            comm_channel->client_counter = ++temp;
            if (n % 2 == 0)
                comm_channel->response_payload.EvenorOdd.val = 0;
            else
                comm_channel->response_payload.EvenorOdd.val = 1;
            comm_channel->is_response_ready = 1;
            request_count++;
            printf("Server has responded to client!! \n");
            printf("Total Number of requests so far is %d\n",request_count);
            pthread_mutex_unlock(&comm_channel->res_mutex);
        }
        break;

        case 3: // prime
        {   printf("Comm Channel is created successfully!! \n");
            printf("Comm Channel for client with key : %d is up and running\n", comm_channel->key);
            while (comm_channel->is_data_ready == 0)
            {
            }
            int n = comm_channel->request_payload.isPrime.n1;
            comm_channel->is_data_ready = 0;

            printf("Recieved request from user with key : %d to check whether %d is prime or not \n", comm_channel->key, n);
            pthread_mutex_lock(&comm_channel->res_mutex);
            comm_channel->client_counter = ++temp;
            comm_channel->request_type = 0;
            if (isPrime(n))
                comm_channel->response_payload.isPrime.val = 0;
            else
                comm_channel->response_payload.isPrime.val = 1;
            comm_channel->is_response_ready = 1;
            request_count++;
            printf("Server has responded to client!! \n");
            printf("Total Number of requests so far is %d\n",request_count);
            pthread_mutex_unlock(&comm_channel->res_mutex);
        }
        break;

        case 4: // negative
        {   printf("Comm Channel is created successfully!! \n");
            printf("Comm Channel for client with key : %d is up and running\n", comm_channel->key);
            int n = comm_channel->request_payload.isNegative.n1;
            printf("Recieved request from user with key : %d to check whether %d is negative or not \n",comm_channel->key, n);
            comm_channel->request_type = 0;
            comm_channel->client_counter = ++temp;
            request_count++;

            printf("Server has responded to client!! \n");
            printf("Total Number of requests so far is %d\n",request_count);
            // if (n < 0)
            //     comm_channel->response_payload.isNegative.val = 0;
            // else
            //     comm_channel->response_payload.isNegative.val = 1;
        }
        break;

        case -1: //disconnect                                                     need to change this from disconnet to unregister, and remove key from server
        {   printf("Comm Channel for client with key : %d is up and running\n", comm_channel->key);
            comm_channel->client_counter = temp;

            printf("Client with key %d unregistered and resources unallocated\n", comm_channel->key);
            printf("-------------------------------------------- \n");

            // Reset request type
            // pthread_mutex_lock(&comm_channel->res_mutex);
            comm_channel->request_type = 0;
            // comm_channel->client_counter = 1;
            // comm_channel->is_response_ready=1;
            // pthread_mutex_unlock(&comm_channel->res_mutex);
        }
        break;

        default:
            break;
        }
    }

    if (munmap(comm_channel, sizeof(client_request)) == -1)
    {
        perror("munmap_comm_channel");
        exit(1);
    }

    close(comm_channel_fd);

}

int main()
{
    int temp = 0;
    int connect_channel_fd = shm_open(CONNECT_CHANNEL_NAME, O_CREAT | O_RDWR, 0666);
   
    
    if (connect_channel_fd == -1)
    {
        perror("shm_open_connect_channel");
        exit(1);
    }

    if (ftruncate(connect_channel_fd, sizeof(client_request)) == -1)
    {
        perror("ftruncate_connect_channel");
        exit(1);
    }

    client_request *connect_channel = (client_request *)mmap(NULL, sizeof(client_request), PROT_READ | PROT_WRITE, MAP_SHARED, connect_channel_fd, 0);
    
    
    if (connect_channel == MAP_FAILED)
    {
        perror("mmap_connect_channel");
        exit(1);
    }
    
    connect_channel->client_counter = 0;
    connect_channel->n_clients = 0;
    connect_channel->is_response_ready = 0;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&connect_channel->mutex, &mutex_attr);

    pthread_mutexattr_t mutex_attr2;
    pthread_mutexattr_init(&mutex_attr2);
    pthread_mutexattr_setpshared(&mutex_attr2, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&connect_channel->res_mutex, &mutex_attr2);

    printf("Server is running...\n");

    pthread_t tids[10];


    while (1)
    {

        if (connect_channel->request_type == 1)
        {   printf("Connect Channel created !! \n");
            if(connect_channel->n_clients!=0){
                    
                for (int i = 0; i < connect_channel->n_clients; i++)
                {
                    if (connect_channel->key == connect_channel->keys[i])
                    {
                        pthread_mutex_lock(&connect_channel->mutex);
                        printf("\nClient with key %d is already connected. Aborting...\n", connect_channel->key);
                        connect_channel->is_response_ready = -1;
                        connect_channel->request_type = 0;
                        connect_channel->key = NULL;
                        
                        pthread_mutex_unlock(&connect_channel->mutex);
                        print_keys(connect_channel->keys, connect_channel->n_clients, connect_channel->is_response_ready);
                        
                        break;
                    }
                }
            }

            if (connect_channel->request_type != 0)
            {
                    pthread_mutex_lock(&connect_channel->mutex);

                    int current_thread = connect_channel->n_clients;

                    // pthread_create(&tids[connect_channel->client_counter], NULL, runner, NULL);
                    pthread_create(&tids[connect_channel->n_clients], NULL, runner, (void*)connect_channel->key);
                    
                    connect_channel->keys[connect_channel->n_clients++] = connect_channel->key;
                    connect_channel->request_type = 0; // Reset request type
                    connect_channel->is_response_ready = 1;
                    // connect_channel->key = NULL;

                    printf("Client with key %d connected.\n", connect_channel->key);

                    // print_keys(connect_channel->keys, connect_channel->n_clients, connect_channel->is_response_ready);

                    pthread_mutex_unlock(&connect_channel->mutex);

                    print_keys(connect_channel->keys, connect_channel->n_clients, connect_channel->is_response_ready);

                    // pthread_join(tids[current_thread], NULL);
            }
        }

        if(connect_channel->request_type == -1){   //unregister

            pthread_mutex_lock(&connect_channel->mutex);
            pthread_mutex_lock(&connect_channel->res_mutex);
            int index;
            for(int i=0; i<connect_channel->n_clients;i++){
                if(connect_channel->keys[i] == connect_channel->key)
                    index = i; 
            }

            for(int i=index; i<connect_channel->n_clients-1;i++){
                connect_channel->keys[i] = connect_channel->keys[i+1];
            }

            connect_channel->n_clients--;

            pthread_mutex_unlock(&connect_channel->mutex);
            pthread_mutex_unlock(&connect_channel->res_mutex);
            
            pthread_mutex_lock(&connect_channel->res_mutex);
            connect_channel->is_response_ready=1;
            connect_channel->request_type=0;
            pthread_mutex_unlock(&connect_channel->res_mutex);

            print_keys(connect_channel->keys, connect_channel->n_clients, connect_channel->is_response_ready);

        }

    }

    if (munmap(connect_channel, sizeof(client_request)) == -1)
    {
        perror("munmap");
        exit(1);
    }

    close(connect_channel_fd);

    return 0;
}

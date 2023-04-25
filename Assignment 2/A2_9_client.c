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
    int request_type; //-1 to disconnect and 1 to connect
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

    int is_data_ready; // flag for client writing

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

int main()
{

    int connect_channel_fd = shm_open(CONNECT_CHANNEL_NAME, O_RDWR, 0666);
    

    if (connect_channel_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    client_request *connect_channel = (client_request *)mmap(NULL, sizeof(client_request), PROT_READ | PROT_WRITE, MAP_SHARED, connect_channel_fd, 0);
    
    if (connect_channel == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    int key;

    while(1){
        
        printf("Enter key for connection: ");
        scanf("%d", &key);

        // Connect to server
        pthread_mutex_lock(&connect_channel->mutex);
        connect_channel->key = key;
        connect_channel->request_type = 1; // 1 for connecting
        pthread_mutex_unlock(&connect_channel->mutex);

        while (connect_channel->is_response_ready == 0)
        {
        }

        printf("Response received: %d\n", connect_channel->is_response_ready);

        if (connect_channel->is_response_ready == -1)
        {
            printf("Key is already used, please try another key\n");
            pthread_mutex_lock(&connect_channel->mutex);
            connect_channel->is_response_ready = 0;
            pthread_mutex_unlock(&connect_channel->mutex);
        }
        else
        {
            printf("Connection established\n");
            pthread_mutex_lock(&connect_channel->mutex);
            connect_channel->is_response_ready = 0;
            pthread_mutex_unlock(&connect_channel->mutex);
            break;
        }
    }

    char comm_channel_name[] = "/comm_channel";
    char buffer[10];
    sprintf(buffer, "%d", key);
    strcat(comm_channel_name, buffer);

    int comm_channel_fd = shm_open(comm_channel_name, O_RDWR, 0666);

    if (comm_channel_fd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    client_request *comm_channel = (client_request *)mmap(NULL, sizeof(client_request), PROT_READ | PROT_WRITE, MAP_SHARED, comm_channel_fd, 0);

    if (comm_channel == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    printf("CLient has connected to server on com channel!!\n");
    printf("-----------------------------------------------\n");

    comm_channel->is_data_ready = 0;
    comm_channel->is_response_ready = 0;
    comm_channel->client_counter = 0;

    // intializing pthread mutex within shared memory segment
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&comm_channel->mutex, &mutex_attr);

    pthread_mutexattr_t mutex_attr2;
    pthread_mutexattr_init(&mutex_attr2);
    pthread_mutexattr_setpshared(&mutex_attr2, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&comm_channel->res_mutex, &mutex_attr2);

    int opt;

    while (1)
    {   
        printf("1. Send Request \n");
        printf("2. Unregister \n");
        printf("Enter choice: ");
        
        scanf("%d", &opt);

        if (opt == 1)
        {
            int choice, ans;
            printf("1. Arithmetic Operation on two numbers\n");
            printf("2. To check if number is even or odd\n");
            printf("3. To check if number is prime\n");
            printf("4. To check if number is negative\n");
            printf("Select operation to be performed \n");
            scanf("%d", &choice);

            switch (choice)
            {
            case 1: // arithemtic
            {
                printf("Enter two numbers: \n");
                int a, b;
                scanf("%d %d", &a, &b);

                pthread_mutex_lock(&comm_channel->mutex);

                comm_channel->request_payload.arithmetic.n1 = a;
                comm_channel->request_payload.arithmetic.n2 = b;
                printf("Select Operation \n");
                printf("1. Addition\n");
                printf("2. Subtraction\n");
                printf("3. Multiplication\n");
                printf("4. Division\n");
                int opr;
                scanf("%d", &opr);
                printf("Client has sent request to server\n");
                printf("-------------------------------------\n");
                switch (opr)
                {
                case 1:
                {
                    comm_channel->request_payload.arithmetic.op = 1;
                }
                break;
                case 2:
                {
                    comm_channel->request_payload.arithmetic.op = 2;
                }
                break;
                case 3:
                {
                    comm_channel->request_payload.arithmetic.op = 3;
                }
                break;
                case 4:
                {
                    comm_channel->request_payload.arithmetic.op = 4;
                }
                break;

                default:
                    printf("Please enter valid option\n");
                    break;
                }

                comm_channel->key = key;
                comm_channel->request_type = 1;
                comm_channel->is_data_ready = 1;

                pthread_mutex_unlock(&comm_channel->mutex);

                // read
                while (comm_channel->is_response_ready == 0)
                {
                }
                int ans = comm_channel->response_payload.arithmetic.ans;
                comm_channel->is_response_ready = 0;

                printf("Server has responded: The arithemtic operation results in %d \n", ans);
            }
            break;

            case 2: // evenorodd
            {
                printf("Enter number\n");
                int num;
                scanf("%d", &num);
                printf("Client has sent request to server\n");
                printf("-------------------------------------\n");

                pthread_mutex_lock(&comm_channel->mutex);

                // write
                comm_channel->request_payload.EvenorOdd.n1 = num;

                comm_channel->key = key;
                comm_channel->request_type = 2;

                comm_channel->is_data_ready = 1;

                pthread_mutex_unlock(&comm_channel->mutex);

                // read

                while (comm_channel->is_response_ready == 0)
                {
                }
                int ans = comm_channel->response_payload.EvenorOdd.val;
                comm_channel->is_response_ready = 0;

                if (ans == 0)
                    printf("Server has responded: Number is even \n");
                else
                    printf("Server has responded: Number is odd \n");
            }
            break;

            case 3: // prime
            {
                printf("Enter number\n");
                int num;
                scanf("%d", &num);
                printf("Client has sent request to server\n");
                printf("-------------------------------------\n");

                pthread_mutex_lock(&comm_channel->mutex);
                comm_channel->request_payload.isPrime.n1 = num;

                comm_channel->key = key;
                comm_channel->request_type = 3;

                comm_channel->is_data_ready = 1;

                pthread_mutex_unlock(&comm_channel->mutex);

                while (comm_channel->is_response_ready == 0)
                {
                }

                int ans = comm_channel->response_payload.isPrime.val;
                comm_channel->is_response_ready = 0;
                if (ans == 0)
                    printf("Server has responded: Number is prime \n");
                else
                    printf("Server has responded: Number is not prime \n");
            }
            break;

            case 4: // negative
            {
                printf("Enter number\n");
                int num;
                scanf("%d", &num);
                printf("Client has sent request to server\n");
                printf("-------------------------------------\n");

                printf("Server has responded: Not supported \n");
                /*
                comm_channel->request_payload.isNegative.n1=num;

                comm_channel->key = key;
                comm_channel->request_type = 4;
                ans = comm_channel->response_payload.isNegative.val;

                /*if(ans==0)
                printf("Number is negative \n");
                else
                printf("Number is not negative \n");*/
            }
            break;

            default:
            {
                printf("Please enter valid option \n");
            }
            break;
            }
        }

        else if (opt == 2)
        {

            // pthread_mutex_lock(&comm_channel->mutex);
            comm_channel->key = key;
            comm_channel->request_type = -1;
            printf("Total number of request made are %d \n", comm_channel->client_counter);

            pthread_mutex_lock(&connect_channel->mutex);
            connect_channel->key = key;
            connect_channel->request_type = -1;
            pthread_mutex_unlock(&connect_channel->mutex);

            while(connect_channel->is_response_ready==0){
            }

            if(connect_channel->is_response_ready==1){
                printf("Successfully Unregistered");
            }

            break;
        }

        else
        {
            printf("Please enter valid option \n");
        }
    }

    if (munmap(connect_channel, sizeof(client_request)) == -1 || munmap(comm_channel, sizeof(client_request)) == -1)
    {
        perror("munmap");
        exit(1);
    }

    close(connect_channel_fd);
    close(comm_channel_fd);

    return 0;
}

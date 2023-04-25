# Operating-Systems-Assignment
Assignments for CS F372 Operating Systems  

 Assignment 1
 : We essentially generate N threads from N worker processes and handle SIGCHILD signal when there is an error.(written in C)  
 
 compile using gcc -o A1_10 A1+10.c -lpthread  
 
 execute using ./A1_10 n a b p x(multiple vals as required for the matrix)  
 enter n a b p and values of x as required
 
 Assignment 2
 : This is a server client implementation using shared memory, mutex locks and multitthreading.(written in C)
 
 compile using gcc server.c -o server -lrt -lpthread
 and for client gcc client.c -o client -lrt -lpthread
 
 execute using ./server and ./client

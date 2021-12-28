// Server Side Code
#include<stdio.h> // for basic input output
#include<stdlib.h> // for exit()
#include<pthread.h> // for pthread APIs
#include<json-c/json.h> // for deserializing data
#include<string.h> 
#include<sys/socket.h> // for socket APIs
#include<sys/types.h>
#include<netinet/in.h> // for server address INADDR_ANY
#include<unistd.h>
#include<stdbool.h> // For boolean
#include <dlfcn.h> // For DLL APIs

#include "function.h" // Include functions defined in function.c
#define MAX 1024
#define BACKLOG 10

// Struct to store information about each thread 
// Thread pool is array of this structure
struct thread_info{
    pthread_t tid; // thread id
    int tno; // thread number
    int running; // 0: not running 1: running
    int fd; // file descriptor for socket
};

// Function to get the index of thread which is available(not running) from given thread pool and also check if the current files and memory are under limit
int get_available_thread_index(struct thread_info thread[], int max_threads, int max_files, int max_memory, bool test, int current_file, int current_memory);

// Function to initialize a server address
void initialise_addr(struct sockaddr_in *server_addr, int port_number);

// Function that defines the start routine of thread. Receives data from client and sends an acknowledgement also calls dll function invoker routine
// @params: pointer to struct thread_info which has information regarding currently executing thread(calling thread)
// return value: void* (none)
void *dispatcher(void *t)
{ 
    ((struct thread_info*)t)->running = 1;
    int client_fd = ((struct thread_info*)t)->fd;
    printf("[+]Success: Thread running client_fd %d\n", client_fd);
    sleep(5);
    char buffer[MAX];
    int read_ret = read(client_fd, buffer, sizeof(buffer));
    if(read_ret == -1)
    {
        perror("[-]Error: Cannot receive data from client\n");
        return NULL;
    }
    printf("[+]Success: Message received: %s\n", buffer);

    char ack_msg[] = "Message Received";
    int write_ret = write(client_fd, ack_msg, sizeof(ack_msg));
    if(write_ret == -1)
    {
        perror("[-]Error: Cannot send data to client\n");
        return NULL;
    }
    printf("[+]Success: Acknowledgement send\n");
    
    int dll_func_ret = dll_func_invoker(buffer);
    if(dll_func_ret == 0)
    {
        printf("[+]Success: DLL function invoking completed\n");    
    }
    else if(dll_func_ret == -1)
    {
        fprintf(stderr, "[-]Error: DLL function invoking failed\n");
    }
    ((struct thread_info*)t)->running = 0;
    printf("[+]Success: Thread execution complete client_fd %d\n", client_fd);
    printf("[+]Success: Closed %d\n\n", client_fd);
    
    fflush(stdin);
    close(client_fd);
    pthread_exit(0);
}

// CLI Arguments Format
// ./[program_name] [port_number] [max_threads] [max_files] [max_memory(KB)]
int main(int argc, char*argv[])
{
    if(argc<5)
    {
        printf("[*]Info: CLI Arguments should be of the form: ./[program_name] [port_number] [max_threads] [max_files] [max_memory(KB)]\n");
        exit(0);
    }
    printf("[+]Success: Server program starting\n");

    // Extract values from the CLI argument
    int port_number = atoi(argv[1]); // Port Number
    int max_threads = atoi(argv[2]); // Max number of threads
    int max_files = atoi(argv[3]); // Max number of files
    int max_memory = atoi(argv[4]); // Max memory

    // Since counting of current file descriptors opened is done via /proc/self/fd. It takes into account some default fd and the socket fd also. So we need to add them to have proper functioning of the program
    // 3 for stdin, stdout, stderr
    // 5 for server socket fd, json tokener parser and rest for safety
    // max_thread for new socket which will create after accept() system call
    max_files = max_files+3+5+max_threads;

    // Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1)
    {
        perror("[-]Error: Server Socket creation failed\n");
        exit(0);
    }
    printf("[+]Success: Server Socket created successfully with file descriptor %d\n", server_fd);  

    // Initialize server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    initialise_addr(&server_addr, port_number);

    // Bind address to the socket
    int bind_ret = bind(server_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr));
    if(bind_ret == -1)
    {
        perror("[-]Binding failed\n");
        exit(0);
    }
    printf("[+]Success: Binding to the address done\n");

    // Listen for connections
    int listen_ret = listen(server_fd, BACKLOG);
    if(listen_ret == -1)
    {
        perror("[-]Error: Failed to listen to port\n");
        exit(0);
    }
    printf("[+]Success: Listening for clients on port: %d\n", port_number);

    // Create a thread pool
    struct thread_info thread[max_threads];
    // Initialize thread pool state
    for(int i=0; i<max_threads; i++)
    {
        thread[i].running=0;
    }
    int index=0; //Index of first available thread which is not running

    while(1)
    {
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        fflush(stdin);

        // Accepting the connections and creating a new socket for communication
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if(client_fd == -1)
        {
            perror("[-]Error: Cannot accept connection from client\n");
            continue;
        }
        printf("[+]Success: Client connected new socket created with fd %d\n", client_fd);
        
        // Function to get the index of thread which is available(not running) from given thread pool and also check if the current files and memory are under limit
        index = get_available_thread_index(thread, max_threads, max_files, max_memory, false, 0, 0);

        // Assign client_fd to this index of thread pool via which communication will take place
        thread[index].fd = client_fd;
        thread[index].tno = index+1;

        // Create Thread and start dispatcher function
        int create_ret = pthread_create(&thread[index].tid, NULL, dispatcher, (void*)&thread[index]);
        if(create_ret == 0)
        {
            printf("[+]Success: Thread Created\n");
            index = (index+1)%max_threads;
        }
        else
        {
            perror("[-]Error: Thread Creation failed\n");
            continue;
        }
        fflush(stdin);

    }
    pthread_exit(NULL);

    // Close Client Socket fd
    int close_ret = close(server_fd);
    if(close_ret == -1)
    {
        perror("[-]Error: Cannot close Server Socket fd\n");
        exit(0);
    }
    printf("[+]Success: Closed Server Socket fd\n");
    return 0;
}

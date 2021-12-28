// Client Side Code
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<json-c/json.h> // for serializing data
#include "function.h" // Include functions defined in function.c
#define MAX 1024

// Function to initialize a server address
void initialise_addr(struct sockaddr_in *server_addr, int port_number);

// CLI Arguments Format
// ./[program_name] [port_number] [dll_name] [function_name] [arguments]
int main(int argc, char*argv[])
{
    if(argc<5)
    {
        printf("[*]Info: CLI Arguments should be of the form: ./[program_name] [port_number] [dll_name] [function_name] [arguments]\n");
        exit(0);
    }
    printf("[+]Success: Client program starting\n");

    // Extract values from the argument
    char dll_name[MAX], func_name[MAX];
    int no_of_args = argc - 4; // Number of Arguments
    char *arg[no_of_args]; 

    int port_number = atoi(argv[1]); // Port Number 
    strcpy(dll_name, argv[2]); // DLL Function Name
    strcpy(func_name, argv[3]); // Function Name
    for(int i=0; i<no_of_args; i++) 
    {
        arg[i] = argv[4+i]; // Arguments
    }

    char buffer[MAX];

    // Function to convert given arguments into JSON object and convert to JSON string
    const char *json_string = form_json_string(dll_name, func_name, no_of_args, arg);
    if(strcpy(buffer, json_string) == NULL)
    {
        perror("[-]Error: Couldn't copy JSON String formed to buffer\n");
        exit(0);
    }

    // Create a socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0)
    {
        perror("[-]Error: Client Socket creation failed\n");
        exit(0);
    }
    printf("[+]Success: Client Socket created successfully with file descriptor %d\n", client_fd);  

    // Initialize server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    initialise_addr(&server_addr, port_number);

    // Connect to server socket
    int connect_ret = connect(client_fd, (struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr));
    if(connect_ret == -1)
    {
        perror("[-]Error: Cannot establish connection between client and server\n");
        exit(0);
    }
    printf("[+]Success: Connection established successfully between client and server\n");

    // Send Data to Server
    int send_ret = write(client_fd, buffer, sizeof(buffer));
    if(send_ret == -1)
    {
        perror("[-]Error: Cannot send data to server socket\n");
        exit(0);
    }
    printf("[+]Success: Message send to server\n");

    // Receive Acknowledgement From Server
    char ack_msg[MAX];
    int recv_ret = recv(client_fd, ack_msg, sizeof(ack_msg), 0);
    if(recv_ret == -1)
    {
        perror("[-]Error: Cannot receive ack meesage from server socket\n");
        exit(0);
    }
    printf("[+]Success: Acknowledgement from server: %s\n", ack_msg);

    // Close Client Socket fd
    int close_ret = close(client_fd);
    if(close_ret == -1)
    {
        perror("[-]Error: Cannot close Client Socket fd\n");
        exit(0);
    }
    printf("[+]Success: Closed Client Socket fd\n");
    return 0;
}
#include<dlfcn.h> // Contains API for DLL
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<json-c/json.h> // For serializing and deserializing data
#include<stdbool.h>
#include<sys/types.h> 
#include<netinet/in.h> // for server address INADDR_ANY
#define MAX 1024

// Struct to store information about each thread 
// Thread pool is array of this structure
struct thread_info{
    pthread_t tid; // thread id
    int tno; // thread number
    int running; // 0: not running 1: running
    int fd; // file descriptor for socket
};

// Function to initialize a server address
// @params: pointer to server address, port number
// return value: void
void initialise_addr(struct sockaddr_in *server_addr, int port_number)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port_number);
    server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
}

// Function to run function with given arguments by loading given DLL 
// @params: pointer to dll name, function name and arguments
// return value: 0 for success, 1 for function not supported, -1 for error
int run_dll_func(const char*dll, const char*func, const char*args[])
{
    void *library;
    char *error;

    library = dlopen(dll, RTLD_LAZY);
    if(!library)
    {
        printf("[-]Error: Cannot open DLL\n");
        return -1;
    }
    // Supports only cos, floor, abs and pow function
    if(strcmp("cos", func) == 0)
    {
        double (*cosine)(double);
        cosine = dlsym(library, func);
        double arg = atof(args[0]);
        double result = (*cosine)(arg);
        printf("[+]Success: Result of cos(%f) = %f\n", arg, result);
    }
    else if(strcmp("floor", func) == 0)
    {
        double (*floor)(double);
        floor = dlsym(library, func);
        double arg = atof(args[0]);
        double result = (*floor)(arg);
        printf("[+]Success: Result of floor(%f) = %f\n", arg, result);
    }
    else if(strcmp("abs", func) == 0)
    {
        int (*abs)(int);
        abs = dlsym(library, func);
        int arg = atoi(args[0]);
        int result = (*abs)(arg);
        printf("[+]Success: Result of abs(%d) = %d\n", arg, result);
    }
    else if(strcmp("pow", func) == 0)
    {
        double (*pow)(double, double);
        pow = dlsym(library, func);
        double arg1 = atof(args[0]);
        double arg2 = atof(args[1]);
        double result = (*pow)(arg1, arg2);
        printf("[+]Success: Result of pow(%f, %f) = %f\n", arg1, arg2, result);
    }
    else
    {
        printf("[*]Info: Supported functions are: floor, cos, pow, abs\n");
        return 1;
    }
    error = dlerror();
    if(error != NULL)
    {
        fprintf(stderr, "[-]Error: Failed to run DLL function %s\n", error);
        return -1;
    }
    
    dlclose(library);
    return 0;
}

// Function to convert given arguments into json object and convert to json string
// @params: pointer to dll name, function name and arguments
// return value: string formed from given arguments
const char * form_json_string(char *dll_name, char *func_name, int no_of_args, char *arg[] )
{
    // Create JSON Object to send the data over socket
    json_object *msg = json_object_new_object();
    // Create JSON String for dll_name
    json_object *j_dll_name = json_object_new_string(dll_name);
    // Create JSON String for func_name
    json_object *j_func_name = json_object_new_string(func_name);
    // Create a JSON array for args
    json_object *j_args = json_object_new_array();
    // Add each argument to this array after converting each argument to individual JSON String
    for(int i=0; i<no_of_args; i++)
    {
        json_object_array_add(j_args, json_object_new_string(arg[i]));
    }
    
    // Form the JSON object msg by adding each key-value pair
    json_object_object_add(msg,"DLL Name", j_dll_name);
    json_object_object_add(msg,"Function Name", j_func_name);
    json_object_object_add(msg,"Arguments", j_args);

    return json_object_to_json_string(msg);
}

// Function to parse data received and call function to execute request 
// @params: char string buffer received from client
// return value: 0 for success, 1 for function not supported, -1 for error
int dll_func_invoker(char buffer[])
{
    // Parse JSON String and extract 
    json_object *parsed_json;
    parsed_json = json_tokener_parse(buffer);

    // Create JSON String for dll_name
    json_object *j_dll_name; 
    // Create JSON String for func_name
    json_object *j_func_name; 
    // Creating a json array
    json_object *j_args;
    json_object_object_get_ex(parsed_json,"DLL Name", &j_dll_name);
    json_object_object_get_ex(parsed_json,"Function Name", &j_func_name);
    json_object_object_get_ex(parsed_json,"Arguments", &j_args);
    
    const char *dll = json_object_get_string(j_dll_name);
    const char *func = json_object_get_string(j_func_name);
    int length = json_object_array_length(j_args);
    const char *args[length];
    for(int i=0; i<length; i++)
    {
        args[i] = json_object_get_string(json_object_array_get_idx(j_args, i));
    }
    int run_ret = run_dll_func(dll, func, args);

    return run_ret;
}

// Function to count number of file descriptors open for calling process by reading from "/proc/self/fd" path
// @params: none
// return value: number of file descriptors
int count_file()
{
    FILE *file;
    char command[MAX];
    char data[MAX];

    // Command to get file descriptor for current process
    sprintf(command, "ls /proc/self/fd/ | wc -l"); 

    // Open pipe for reading and execute the command
    file = popen(command,"r"); 

    // Get the data from the command executed
    fgets(data, MAX, file);
    int fd = atoi(data);

    if (pclose(file) != 0)
    {
        fprintf(stderr,"[-]Error: Failed to close pipe\n");
    }
    return fd;
}

// Function to count memory used by calling process by reading VmRSS parameter from "/proc/self/status" path
// @params: none
// return value: memory in KB 
int count_memory()
{
    FILE* file; 
    char buffer[MAX];
    char data[MAX];

    // Open file having the process info
    file = fopen("/proc/self/status", "r");

    // Read the file and find VmRSS parameter
    while (fscanf(file, " %1023s", buffer) == 1) 
    {
        if (strcmp(buffer, "VmRSS:") == 0) 
        {
            fgets(data, MAX, file);
        }
    }
    fclose(file);

    int memory = atoi(data);
    return memory; 
    // unsigned long size,resident,share,text,lib,data,dt;
    // file = fopen("/proc/self/statm", "r");
    // if(7 != fscanf(file,"%ld %ld %ld %ld %ld %ld %ld",&size,&resident,&share,&text,&lib,&data,&dt))
    // {
    //     perror("statm");
    //     // abort();
    // }
    // return resident;
}

// Function to get the index of thread which is available(not running) from given thread pool and also check if the current files and memory are under limit
// @params: thread pool which is array of structure thread_info
//          maximum threads allowed
//          maximum files allowed
//          maximum memory allowed
//          boolean value: false if function called from server true: if called from test function
//          current files opened in process
//          current memory used in process
// return value: index of the first thread available
int get_available_thread_index(struct thread_info thread[], int max_threads, int max_files, int max_memory, bool test, int current_file, int current_memory)
{
    bool thread_found = false, file_under_limit = true, mem_under_limit = true;
    int index;
    while(thread_found == false && file_under_limit == true && mem_under_limit == true)
    {
        thread_found = false;
        for(int i=0; i<max_threads; i++) // Traverse over each thread in thread pool
        {
            if(thread[i].running == 0) // If not running
            {
                index=i;
                thread_found=true;
                // check file
                if(!test)
                current_file = count_file(); // Get Current Files Opened

                printf("[*]Info: Current Files: %d\n", current_file);
                if(current_file >= max_files) // If files opened are greater than the maximum limit then wait till files get under limit
                file_under_limit = false;

                // check memory
                if(!test)
                current_memory = count_memory(); // Get Memory used by the process

                printf("[*]Info: Current Memory: %d\n", current_memory);
                if(current_memory >= max_memory) // If memory used by the process is greater than the maximum limit then wait till memory get under limit
                mem_under_limit = false;

                break;
            }
        }
        if(!thread_found)
        {
            printf("[*]Info: Thread Limit Exceeded. Waiting for a thread to be free from given pool\n");
        }
        if(!file_under_limit)
        {
            thread_found = false;
            printf("[*]Info: File Limit Exceeded. Waiting for a thread to free files\n");
        }
        if(!mem_under_limit)
        {
            thread_found = false;
            printf("[*]Info: Memory Limit Exceeded. Waiting for a thread to free memory\n");
        }
    }
    return index;
}


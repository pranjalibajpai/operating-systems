#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdbool.h>
#include "function.h"

#define MAX 1024

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

// Function to test execution of dll_func_invoker()
// About dll_func_invoker()
// Function to parse data received and call function to execute request 
// @params: char string buffer received from client
// return value: 0 for success, 1 for function not supported, -1 for error
void test_dll_func_invoker()
{
    int return_val;
    char buf[1024];
    printf("*********************Testing dll_func_invoker*********************\n\n");
    
    // Checking "cos" function from libm.so.6
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.6", "cos", 1, (char*[]){"1.2"}));
    return_val = dll_func_invoker(buf);
    if(return_val == 0)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    
    // Checking "floor" function from libm.so.6
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.6", "floor", 1, (char*[]){"4.9"}));
    return_val = dll_func_invoker(buf);
    if(return_val == 0)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }

    // Checking "abs" function from libm.so.6
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.6", "abs", 1, (char*[]){"-16"}));
    return_val = dll_func_invoker(buf);
    if(return_val == 0)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }

    // Checking "pow" function from libm.so.6
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.6", "pow", 2, (char*[]){"3", "2"}));
    return_val = dll_func_invoker(buf);
    if(return_val == 0)
    {
        printf("[+]Test Case 4: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 4: Failed\n\n");
    }

    // Checking "floor" function from libm.so.6 but with wrong DLL name
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.", "cos", 1, (char*[]){"1.2"}));
    return_val = dll_func_invoker(buf);
    if(return_val == -1)
    {
        printf("[+]Test Case 5: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 5: Failed\n\n");
    }

    // Checking unsupported function "sin"
    strcpy(buf, form_json_string("/lib/x86_64-linux-gnu/libm.so.6", "sin", 1, (char*[]){"1.2"}));
    return_val = dll_func_invoker(buf);
    if(return_val == 1)
    {
        printf("[+]Test Case 6: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 6: Failed\n\n");
    }
}

// Function to test execution of run_dll_func()
// About run_dll_func()
// Function to run function with given arguments by loading given DLL 
// @params: pointer to dll name, function name and arguments
// return value: 0 for success, 1 for function not supported, -1 for error
void test_run_dll_func()
{
    int return_val;
    printf("*********************Testing run_dll_func*********************\n\n");
    
    // Checking "cos" function from libm.so.6
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.6", "cos", (const char*[]){"1.2"});
    if(return_val == 0)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }

    // Checking "floor" function from libm.so.6
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.6", "floor", (const char*[]){"4.7"});
    if(return_val == 0)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }

    // Checking "abs" function from libm.so.6
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.6", "abs", (const char*[]){"-12"});
    if(return_val == 0)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }

    // Checking "pow" function from libm.so.6
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.6", "pow", (const char*[]){"3", "4"});
    if(return_val == 0)
    {
        printf("[+]Test Case 4: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 4: Failed\n\n");
    }

    // Checking "cos" function from libm.so.6 with wrong DLL name
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.", "cos", (const char*[]){"1.2"});
    if(return_val == -1)
    {
        printf("[+]Test Case 5: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 5: Failed\n\n");
    }

    // Checking unsupported function "sin"
    return_val = run_dll_func("/lib/x86_64-linux-gnu/libm.so.6", "sin", (const char*[]){"1.2"});
    if(return_val == 1)
    {
        printf("[+]Test Case 6: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 6: Failed\n\n");
    }
}

// Function to test execution of test_get_available_thread_index()
// About test_get_available_thread_index()
// Function to get the index of thread which is available(not running) from given thread pool and also check if the current files and memory are under limit
// @params: thread pool which is array of structure thread_info
//          maximum threads allowed
//          maximum files allowed
//          maximum memory allowed
//          boolean value: false if function called from server true: if called from test function
//          current files opened in process
//          current memory used in process
// return value: index of the first thread available
void test_get_available_thread_index()
{
    int index;
    char buf[1024];
    printf("*********************Testing get_available_thread_index*********************\n\n");
    struct thread_info thread[4];
    for(int i=0; i<3; i++)
    {
        thread[i].running = 1;
    }
    thread[3].running = 0;

    // First thread available at index 3. File & Memory under limit
    index = get_available_thread_index(thread, 4, 7, 1000, true, 4, 900);
    if(index == 3)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    thread[0].running = 0;

    // First thread available at index 0. File & Memory under limit
    index = get_available_thread_index(thread, 4, 7, 1000, true, 4, 900);
    if(index == 0)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }

    // First thread available at index 0. File under limit but Memory over limit
    index = get_available_thread_index(thread, 4, 7, 1000, true, 4, 1100);
    if(index == 0)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }

    // First thread available at index 0. File over limit but Memory under limit
    index = get_available_thread_index(thread, 4, 7, 1000, true, 8, 900);
    if(index == 0)
    {
        printf("[+]Test Case 4: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 4: Failed\n\n");
    }

    // First thread available at index 0. File & Memory over limit
    index = get_available_thread_index(thread, 4, 7, 1000, true, 8, 1000);
    if(index == 0)
    {
        printf("[+]Test Case 5: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 5: Failed\n\n");
    }
}

int main()
{
    test_dll_func_invoker();
    test_run_dll_func();
    test_get_available_thread_index();
    return 0;
}
CS303 Assignment 1

Submitter name: Pranjali Bajpai

Roll No.: 2018EEB1243

=================================


1. About
    The program depicts how a multithreaded socket server works. How the various clients are handled. It also involves invoking Dynamically Loaded Libraries. 
    The essence of the program is: client sends a request to the server. Upon receiving the requests, the server allots a particular thread from the thread pool according to the constraints
    which are specified in the CLI arguments, number of threads, number of files, amount of memory and executes the request of the client.

2. A description of how this program works (i.e. its logic)

    a. Basic Flow of the Code
        1. Server Side[Multithreaded Socket Server]
            |_Create a server socket, bind it to address, and start listening for connection
            |_Create a thread pool of size specified in CLI arguments and initialise state of each thread as not running 
            |_When client gets connected invoke the function to get the index of thread which is available
            |_In case no thread is available it will wait (due to outer while loop) till any other thread stops running and is available
            |_Also check the file and memory limit. In case the current file limit or the memory limit is exceeding the maximum limit specified in the CLI arguments, it will wait again(due to iuter while loop) till any of the thread closes files(in case of File limit exceeding) and frees up memory(in case of Memory Limit Exceeding)
            |_Else if thread is available and memory and file limits are met, thread will be created and and executed
            |_For checking these limits /proc file system is used, specifically /proc/self/fd for files and /proc/self/status[VmRSS parameters] for memory
            |_For Each client a new thread spawns and the start routine of the thread starts executing(function named dispatcher).
            |_The function first sets the state of the current thread as running and then message is received from client and acknowledgement is sent upon successfully receiving of the message
            |_Next, the message received in the form of JSON string is parsed and dll name, function name and arguments are extracted from it via APIs from JSON-C library
            |_Then function is invoked and result is obtained
            |_After all execution, the state of the thread is again set to not running.
        2. Client Side
            |_First make JSON object from the CLI arguments and convert it to JSON string
            |_Create a client socket, initialize server address and connect to server socket
            |_Upon successfull connection, send the JSON string to the server
            |_Wait for acknowledgement message to be received from server
            |_Print the relevant details and exit

    b. Supported library
        The program currently supports only following stanadard DLL
            |_math library (/lib/x86_64-linux-gnu/libm.so.6)

    c. Supported Function
        The program currently supports following functions
            |_cos
            |_floor
            |_abs
            |_pow

3. How to compile and run this program

    a. File Structure
        |_output // Contains output when tested
            |_client.txt
            |_server.txt
            |_test.txt
        |_src
            |_client.c 
            |_function.c // Common File which includes implementation of various functions of client and server. It is used by client.c, server.c and test.c. It is created so that we do not copy paste the function code for testing
            |_function.h
            |_server.c
            |_test.c
        |_Readme.txt
        |_Screenshots.pdf

    b. Pre-Installation
        # Install JSON-C library
        1. sudo apt update
        2. sudo apt install libjson-c-dev 

    c. Compiling and Running the main program
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-1/src
        3. Start the server 
                gcc server.c function.c -o server -pthread -ldl -ljson-c
                ./server [port_number] [max_threads] [max_files] [max_memory(KB)]

                example: ./server 4500 8 10 1000

        4. Start the Client 
                gcc client.c function.c -o client -ljson-c -ldl
                ./client [port_number] [dll_name] [function_name] [arguments]

                example 1: ./client 4500 /lib/x86_64-linux-gnu/libm.so.6 cos 1.2
                example 2: ./client 4500 /lib/x86_64-linux-gnu/libm.so.6 pow 3 4 

    d. Compiling and Running Tests
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-1/src
        3. Start the execution of test program
                gcc test_main.c function.c -o test -pthread -ljson-c -ldl
                ./test       

    e. In case some error occurs, please check the following
        1. In case of error: "Binding failed: Address already in use" Make sure to change the port number as this is the common error that comes. Using higher port numbers which are free generally avoids the error.
        2. Make sure to pass only functions mentioned in the supported function & DLL List(although relevant errors will be printed).
        3. Change the path of DLL file according to your system and provide full path like this: "/lib/x86_64-linux-gnu/libm.so.6".
        4. In case no output shows up, in the server side, it will be shown at the end or by pressing ctrl+z. Although I have used fflush still sometimes this problem occurs.
4. Screenshots
    Please refer cd CSL303-2018EEB1243-1/_Screenshots.pdf

5. References
    a. For JSON-C: 
        1. https://github.com/json-c/json-c/wiki/List-of-json-c-tutorials
    b. For File and Memory Limits:
        1. /proc/self/fd and /proc/self/status https://man7.org/linux/man-pages/man5/proc.5.html
        2. Executing Linux Command and get Output https://stackoverflow.com/questions/4757512/execute-a-linux-command-in-the-c-program
        3. https://devconnected.com/how-to-count-files-in-directory-on-linux/
        4. https://stackoverflow.com/questions/1558402/memory-usage-of-current-process-in-c
    c. Compilation of multiple Source files
        1. https://www.linuxtopia.org/online_books/an_introduction_to_gcc/gccintro_11.html 
    d. Functions in math.h 
        1. https://www.tutorialspoint.com/c_standard_library/math_h.htm
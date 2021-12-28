CS303 Assignment 2

Submitter name: Pranjali Bajpai

Roll No.: 2018EEB1243

=================================


1. About
    The program depicts detection of deadlocks by a separate thread in a system where multiple threads are running and requesting random resources one type at a time
    and how to reolve deadlocks by applying various heuristics for terminating the threads involved in deadlock. The program also finds the average time between 
    occurence of deadlocks for various heuristics.

2. A description of how this program works (i.e. its logic)

    a. Basic Flow of the Code
        |_Create a thread pool of size max_threads(specified in CLI Arguments) and initialize their values and start executing them.
        |_Create another thread which will be used for detecting the deadlock and start executing it.
        |_Worker Thread
            |_Mark the state of thread as running.
            |_Decide random set of resources for the thread
            |_Repeat until thread acquires all the desired resource
                |_Randomly decide the resource type which thread wants to request.
                |_From the remaining number of resources that thread needs of this resource type, Decide a random number and request it
                |_If remaining comes out to be zero then again decide another resource type randomly.
                |_If the request made can be fulfilled, i.e. request<available, then allocate resource successfully
                |_Else allocate whatever instances are available for that resource type and hold them until it gets the complete set
            |_Once thread acquired all resources, release the resources and again repeat the steps
        |_There is a mutex variable which is locked while allocating and deallocating resources, as they are shared variable among all threads.
        |_Deadlock Detector Thread
            |_Make a sleep of time interval equal to deadlock detection interval specified in CLI Argument.
            |_Acquire the same mutex lock which is used for allocation/deallocation as we do not want the current status of resources to change while deadlock 
                detection algorithm is running
            |_Initialize the array and matrix used in the algorithm.
                |_Deadlock Detection algorithm
                    |_Find an index of a thread which can be executed with the current set of available resources
                    |_If index exists, release the resources allocated to the thread and mark the thread as deadlock free
                    |_If no index exists break the loop and check the boolean array if there is a thread available which is not completed its execution.
                    |_Print the list of threads which cannot be executed(These are the deadlocked threads).
                    |_If deadlocked threads found, find the time at which it occurs using "gettimeofday()" function
                    |_Now, terminate the thread according to the heuristic supplied in CLI Argument and end the algorithm
                    |_Check the count of number of times the algorithm is run if it exceeds the max_count(specified in CLI Argument) terminate the program after 
                        finding the average time between deadlocks and printing it.
                    |_Release the lock acquired in the end.
                |_Heuristics Used for Terminating Deadlocked Worker Threads
                    |_H1: Terminate deadlocked thread with maximum allocated resources
                    |_H2: Terminate deadlocked thread with minimum allocated resources
                    |_H3: Terminate thread which holds maximum instances of that resource which is needed maximum by all deadlocked threads combined
                    |_H4: Terminate first thread which is deadlocked
                    |_H5: Terminate all deadlocked threads at once
            |_In case of single thread, no dealdock will be detected.

    c. Assumptions
        |_Resource name is taken as character so maximum resources that can be in the program is 52(A-Z, a-z).
        |_Number of resources available for each resource_type id greater than zero.
        |_srand() is used with seed value as current time in main(), so that rand() produces distinct set of random values
        |_MAX_PAUSE is defined in the program which is the maximum pause between requests.
        |_The program will run until the count of the numbe rof times deadlock detection algorithm runs is less than the max value specified in CLI Argument

3. How to compile and run this program

    a. File Structure
        |_output // Contains output when tested
            |_main_h1.txt // For heuristic H1
            |_main_h2.txt // For heuristic H2
            |_main_h3.txt // For heuristic H3
            |_main_h4.txt // For heuristic H4
            |_main_h5.txt // For heuristic H5
            |_test.txt
        |_src
            |_main.c 
            |_test.c
        |_Readme.txt

    b. Compiling and Running the main program
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-2/src
        3. Start the main program 
                gcc main.c -o main -pthread
                ./[program_name] [number_of_resources] [name_of resources] [count_of_each_resource] [max_threads] [deadlock_detection_interval(in seconds)] [heuristic_type] [number_of_times_detect_deadlock_algorithm_will_run]
                heuristic_type = 1 for H1, 2 for H2, 3 for H3, 4 for H4, 5 for H5
                
                example: ./main 2 A B 10 10 3 4 1 5
                
    d. Compiling and Running Tests
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-2/src
        3. Start the execution of test program
                gcc test -o test -pthread
                ./test       

    e. In case some error occurs, please check the following
        1. In case no output shows up, it will be shown at the end or by pressing ctrl+z. Although I have used fflush still sometimes this problem occurs.

4. Observations

    Time taken by the program for
    a. H1: 10.000989 seconds [Terminate deadlocked thread with maximum allocated resources]
    b. H2: 6.667324 seconds [Terminate deadlocked thread with minimum allocated resources]
    c. H3: 10.000982 seconds [Terminate thread which holds maximum instances of that resource which is needed maximum by all deadlocked threads combined]
    d. H4: 4.000410 seconds [Terminate first thread which is deadlocked]
    e. H5: 5.000558 seconds [Terminate all deadlocked threads at once]

5. References
    a. Operating System Internals And Design by William Stallings 7th Edition
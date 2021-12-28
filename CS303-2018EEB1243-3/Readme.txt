CS303 Assignment 3

Submitter name: Pranjali Bajpai

Roll No.: 2018EEB1243

=================================


1. About
    The program depicts various placement algorithm for dynamic partition method, namely first fit, best fit and next fit and measures the memory utilization and 
    average turnaround time to observe the performance for all the three algorithms.

2. A description of how this program works (i.e. its logic)

    a. Basic Flow of the Code
        |_Extract the parameters and find the process arrival time
        |_Find the maximum processes that can arrive in the worst case
        |_Max process=process arrival rate, r * total time, T
        |_Assume memory is in blocks of 10MB, i.e. each index represents 10MB
        |_Assume memory required by OS is reserved at initial indexes
        |_Initialise memory and process_info for first fit, next fit and best fit
        |_Since we know the process arrival rate, we know the arrival time of each process (=i/r)
        |_Initialise a variable time to 0
        |_Loop till time  < total duration
            |_Deallocate memory for the process whose end time > current time all 3 algorithms
            |_Try allocating process which can be allocated at the current time with current memory for 3 algorithms respectively
        |_Find statistics and print them for each algorithm 

    b. Note
        |_There is an upper limit on number of processes which depends on the arrival rate of the process.
        |_The time duration at which steady state occurs also depends on process arrival rate and other parameters. 
        |_It may be the case for same parameters, the result(memory utilization and turnaround time) will be different when executed more than one time.

3. How to compile and run this program

    a. File Structure
        |_src
            |_main.c 
        |_tests
            |_test.c 
            |_test_output.txt
        |_Observations.txt
        |_Readme.txt

    b. Compiling and Running the main program
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-3/src
        3. Start the main program 
                gcc main.c -o main 
                ./[program_name] [p] [q] [n] [m] [t] [T in minutes]

                example: ./main 1000 200 10 10 10 200
                
    d. Compiling and Running Tests
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-3/tests
        3. Start the execution of test program
                gcc test -o test 
                ./test       

4. Snapshots

pranjali@DESKTOP-PE8R3SN:~/CS303-2018EEB1243-3/src$ gcc main.c -o main
pranjali@DESKTOP-PE8R3SN:~/CS303-2018EEB1243-3/src$ ./main 1000 200 10 10 10 200

                        FIRST_FIT               NEXT_FIT                BEST_FIT
Memory Utilization      86.000000               94.000000               98.000000
Average Turnaround      4609.427166             4616.085544             4571.843012

5. Observations And Results
    Please refer CSL303-2018EEB1243-3/Observations.txt
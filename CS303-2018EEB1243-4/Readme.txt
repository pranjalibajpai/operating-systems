Assignment-4

Submitter name: Pranjali Bajpai

Roll No.: 2018EEB1243

Course: CS303

=================================


1. About
    The program depicts various scheduling algorithms namely, Random, FIFO, SSTF, SCAN, CSCAN for scheduling set of disk requests and compares 
    their performance by finding the response time and throughput for different values of rotation time. The program works by generating a fixed set of random
    requests at once and then they are sent to different algorithm scheduler for scheduling.
    Random: Requests are scheduled in an random order. 
    FIFO stands for First In First Out: Requests are scheduled in the order as and when they arrived. 
    SSTF stands for Shortest Service Time First: Requests are scheduled in the order of their increasing seek time. 
    SCAN: The read/write head is moved in one direction(towards right) executing the requests which come in the way and when the head reaches the end 
    again start moving in the backward direction(towards left) executing the requests which come in the way. 
    CSCAN stands for Circular SCAN: The read/write head is moved in one direction(towards right) executing the requests which come in the way and when 
    the head reaches the end again start from zero in moving again towards right executing the requests which come in the way. 

2. A description of how this program works
    a. The program starts by initilaizing the data structures used for read/write hea dpositions, requests and stats for various algorithms.
    b. There are four platters and four read/write heads which move independently, i.e. each one of the head could be at different cylinder at any time. The disk is rotating at constant speed specified in CLI arguments
    c. Generating Random requests
       |_The structure used to store the request information includes id, cylinder number, sector number, number of sectors requests.
       |_An 2d array of this structure size number of rows=MAX_PLATTER and columns=MAX_REQUESTS is created. 
       |_Each row will contain requests corresponding to its platter.
       |_Id is sequential. First platter number is generated randomly. Then, cylinder number, sector number and number of sector numbers are found randomly.
       |_At index=platter number, this request is appended.
       
    d. The working of all the algorithms is as follows:
        |_FIFO: Execute the requests in the same order as they are in the request array
        |_Random: First find a random ordering of index of requests and then execute them in that order.
        |_SSTF: For each iteration, find the request with shortest service time, execute it, update the read write head and repeat the same untill all requests are executed
        |_SCAN & CSCAN: First update the Read write head to a random value then find the SCAN/CSCAN ordering of index of requests and then execute them in that order. 
            /* The requests are executed independently for each platter */
            /* For Random, FIFO, SSTF the position are initialized to zero and for SCAN, CSCAN their starting positions are randomly assigned */
    e. The response time is calculated as follows
        Response Time=Seek Time+Rotational Delay+Data Transfer Time
        |_Seek time: time to move the head to required cylinder
            Given average seek time is assumed to be the average seek time between consecutive cylinders to get better insight on performance of the algorithms 
        |_Rotational Delay: time to move the head to required sector(half the revolution time)
        |_Data Transfer time: time taken by the head to move requested number of sectors
    f. The throughput is found by dividing the total number of requests by time taken to execute last request
        |_Since the heads are independent, therefore the requests which are of different platters are executed simultaneously and independently
        |_So, first sum of response time for each platter requests is calculated and then the maximum among all 4 equals the time taken to finish all requests(time at which last request finishes)
    
3. How to compile and run this program
    a. File Structure
        |_results 
            |_output.txt
            |_test.txt
            |_observations.txt
        |_src
            |_function.c
            |_function.h
            |_main.c 
            |_test.c
        |_Readme.txt

    b. Compiling and Running the main program
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-4/src
        3. Start the main program 
                gcc main.c function.c -o main -lm
                ./[program_name] [revolutions/minute(r)] [sector size(n)] [average seek time(Ts)]
                
                example: ./main 15000 512 4
                
    d. Compiling and Running Tests
        1. Unzip the folder
        2. cd CSL303-2018EEB1243-4/src
        3. Start the execution of test program
                 gcc test.c function.c -o test -lm
                ./test 
    /* -lm is used for math.h[pow and sqrt are used for finding standard deviation] */   

4. Note
    a. Cylinders and Sectors are based on zero indexing
    b. Number of requested sectors have an upper limit, i.e. [0, MAX_SECTOR]
    c. The variable MAX_REQUEST can be changed for changing number of requests for which the simulation will take place.
       However make sure to change the variable in all three files[main.c, function.c, test.c] to avoid any error. The value of the variable in all 3 files should be same.
       Same goes with MAX_SECTOR, MAX_PLATTER and MAX_CYLINDER.
    d. The request in output are printed in a way so that requests of platter are displayed consecutively. 
       In reality, the platter number is random.

5. Snapshots
    Please Refer 
        a. CS303-2018EEB1243-4/results/output.txt for main program output
        b. CS303-2018EEB1243-4/results/test.txt for test program output

6. Observations
    Please Refer CS303-2018EEB1243-4/results/observations.txt

7. References
    a. Operating System Internals And Design by William Stallings 7th Edition
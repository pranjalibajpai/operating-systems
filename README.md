# operating-systems

##  [Mutithreaded Socket Server Based Thread Dispatcher](https://github.com/pranjalibajpai/operating-systems/tree/main/CS303-2018EEB1243-1)
The program depicts how a multithreaded socket server works. How the various clients are handled. It also involves invoking Dynamically Loaded Libraries. The essence of the program is: client sends a request to the server. Upon receiving the requests, the server allots a particular thread from the thread pool according to the constraints which are specified in the CLI arguments, number of threads, number of files, amount of memory and executes the request of the client.
## [Deadlock Detection ](https://github.com/pranjalibajpai/operating-systems/tree/main/CS303-2018EEB1243-2)
The program depicts detection of deadlocks by a separate thread in a system where multiple threads are running and requesting random resources one type at a time and how to reolve deadlocks by applying verious heuristics for terminating the threads involved in deadlock. The program also finds the average time between occurence of deadlocks for various heuristics.
## [Simulation Of Placement Algorithms for Dynamic Partitioning](https://github.com/pranjalibajpai/operating-systems/tree/main/CS303-2018EEB1243-3)
The program depicts various placement algorithm for dynamic partition method, namely first fit, best fit and next fit and measures the memory utilization and average turnaround time to observe the performance for all the three algorithms.
## [Simulation of Disk Scheduling Algorithms](https://github.com/pranjalibajpai/operating-systems/tree/main/CS303-2018EEB1243-4)
The program depicts various scheduling algorithms namely, Random, FIFO, SSTF, SCAN, CSCAN for scheduling set of disk requests and compares 
their performance by finding the response time and throughput for different values of rotation time. The program works by generating afixed set of random requests at once and then they are sent to different algorithm scheduler for scheduling.
- Random: Requests are scheduled in an random order. 
- FIFO stands for First In First Out: Requests are scheduled in the order as and when they arrived. 
- SSTF stands for Shortest Service Time First: Requests are scheduled in the order of their increasing seek time. 
- SCAN: The read/write head is moved in one direction(towards right) executing the requests which come in the way and when the head reaches the end again start moving in the backward direction(towards left) executing the requests which come in the way. 
- CSCAN stands for Circular SCAN: The read/write head is moved in one direction(towards right) executing the requests which come in the way and when the head reaches the end again start from zero in moving again towards right executing the requests which come in the way. 
## [FUSE based FS](https://github.com/pranjalibajpai/operating-systems/tree/main/CS303-2018EEB1243-5)
The program depicts a FUSE based file system which treats the email account as a storage disk. FUSE stands for File System in User Space which allows to create a file system in user space and without requiring kernel privileges. All the folders in the email account are mapped to the directory on which email account is mounted. Each email is treated as a file. The subject of the email is treated as name of the file and the body of email represents the content of the files. The file system based commands like
- cd : Change directory
- mkdir: Make directory
- ls: List the content of directory
- rmdir: Remove directory
- echo <TEXT> >> file : Create a file and write the content to it
- mv: Rename a directory
can be executed on the mounted directory.
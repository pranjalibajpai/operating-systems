#include<stdio.h>
#include<stdlib.h> // for malloc(), rand()
#include<unistd.h> // for sleep
#include<stdbool.h> // for bool
#include<pthread.h> // for pthread APIs
#include<time.h> // for time
#include<sys/time.h> // for gettimeofday()
#define MAX_PAUSE 4 // Max pause(in seconds) between making request by thread

// Structure to store resource info namely, resource name and count
struct resource_info
{
    char name; // Name Assigned to resource
    int count; // Total Count of Resource
    int allocated; // Total Allocated Resource
};

// Structure to store thread info
struct thread_info
{
    pthread_t t_id; // Thread id
    int index; // Index of thread pool
    struct resource_info* t_res; // Resource info 
    bool running; // True: running False: Not running
};

// Global variables
int n; // Number of type of resources
struct resource_info *resource; // Resource info
int max_threads; // Max threads allowed
int deadlock_detection_interval; // Deadlock Detection Interval
pthread_mutex_t mutex; // Mutex variable
struct thread_info *thread; // Thread Pool
int max_count; // Max number of times deadlock detection algorithm will run
int heuristic; // Heuristic for terminating the deadlocked worker thread
struct timeval *deadlock_time; // Store time between every deadlock
bool *d; // Boolean array True: deadlock occur False: No deadlock detected

// Function to check whether current thread needs more resource or not
// @params: pointer to thread_info struct for that thread, number of resources
// return value: true: if need more resources false: request is fulfilled for that thread
bool need_resource(struct thread_info* t)
{
    bool need=false;
    for(int i=0; i<n; i++)
    {
        if(t->t_res[i].count != t->t_res[i].allocated)
        {
            need=true;
            break;
        }
    }
    return need;
}

// Function to print current resource status
// @params: void
// return value: void
void print_current_resource_status()
{
    printf("***********************RESOURCE STATUS(Name, Remaining): ");
    for(int i=0; i<n; i++)
    {
        printf("%c=%d ", resource[i].name, resource[i].count-resource[i].allocated);
    }
    printf("***********************\n\n");
}

// Function to print current thread resource status
// @params: pointer to struct thread_info for that thread
// return value: void
void print_current_thread_status(struct thread_info* t)
{
    printf("[Thread %d]: RESOURCE STATUS(Name, Need): ", ((struct thread_info*)t)->index);
    for(int i=0; i<n; i++)
    {
        printf("%c=%d ", resource[i].name, t->t_res[i].count-t->t_res[i].allocated);
    }
    printf("\n\n");
}

// Function to allocate required amount of a particular resource type to a thread
// @params: pointer to struct thread_info for that thread, resource_type, number of instances required 
// return value: true: successfull allocation false: unsuccessful allocation
bool allocate(struct thread_info* t, int resource_type, int required)
{
    bool success=true;
    if(t->running==false)
    return true;
    
    // Acquire mutex lock
    pthread_mutex_lock(&mutex);

    int available=resource[resource_type].count-resource[resource_type].allocated; 
    if((required) <= available)
    {
        t->t_res[resource_type].allocated+=(required);
        resource[resource_type].allocated+=(required);
        printf("[Thread %d]: Resource %c allocated\n", t->index, resource[resource_type].name);
    }
    else if(available==0)
    {
        printf("[Thread %d]: No instances of Resource %c available. Waiting for %c.\n", t->index, resource[resource_type].name, resource[resource_type].name);
        success=false;
    }
    else
    {
        printf("[Thread %d]: Allocated %d instances of Resource %c. Waiting for %c\n", t->index, available, resource[resource_type].name, resource[resource_type].name);
        t->t_res[resource_type].allocated+=available;
        resource[resource_type].allocated+=available;
        required-=available;
        success=false;
    }
    print_current_resource_status();
    print_current_thread_status(t);

    // Release lock
    pthread_mutex_unlock(&mutex);
    return success;
}

// Function to release resources held by a particular thread
// @params: pointer to struct thread_info for that thread
// return value: void
void deallocate(struct thread_info* t)
{
    // Acquire lock
    pthread_mutex_lock(&mutex);
    for(int i=0; i<n; i++)
    {
        resource[i].allocated=resource[i].allocated-t->t_res[i].allocated;
        t->t_res[i].allocated=0;
        t->t_res[i].count=0;
    }
    // Release Lock
    pthread_mutex_unlock(&mutex);
}

// Function to genrate a random floating point number between [0.7, 1.5]
// @params: void
// return value: floating point number in [0.7, 1.5]
float random_float()
{
    return 0.7+((float)rand()/RAND_MAX)*(1.5-0.7);
}

// Function to decide random set of resources required by a particular thread
// @params: pointer to struct thread_info for that thread
// return value: void
void decide_resource_set(struct thread_info* t)
{   
    // Acquire Lock
    pthread_mutex_lock(&mutex);
    for(int i=0; i<n; i++)
    {
        t->t_res[i].count=rand()%resource[i].count+1;
        printf("[Thread %d]: Need resource %c = %d\n", t->index, resource[i].name, t->t_res[i].count);
    }
    // Release Lock
    pthread_mutex_unlock(&mutex);
}

// Start routine for threads 
// @params: pointer to struct thread_info for that thread
// return value: void
void *execute(void *thread)
{
    while(1)
    {
        ((struct thread_info*)thread)->running=true;
        printf("[Thread %d]: RUNNING\n", ((struct thread_info*)thread)->index);
        
        // Decide random set of resources for the thread
        decide_resource_set((struct thread_info*)thread);  

        while(need_resource((struct thread_info*)thread)==true && ((struct thread_info*)thread)->running==true)
        {
            // Randomly decide the resource type
            int resource_type=rand()%n, remaining=0; 

            // Find remaining resources required by thread of resource_type
            if(((struct thread_info*)thread)->running==true)
            {
                remaining=((struct thread_info*)thread)->t_res[resource_type].count-((struct thread_info*)thread)->t_res[resource_type].allocated;
            }
            
            // If thread no more requires any instance of the resource "resource_type" 
            if(remaining==0)
                continue;
            
            // Find Random number of resource
            int resource_number=rand()%remaining+1;

            // Before requesting check if thread is running still or not
            if(((struct thread_info*)thread)->running==false)
            break;

            printf("[Thread %d]: Requesting resource %c = %d\n", ((struct thread_info*)thread)->index, resource[resource_type].name, resource_number);
            
            // Check whether request can be fulfilled or not
            while(allocate((struct thread_info*)thread, resource_type, resource_number)==false && ((struct thread_info*)thread)->running==true)
            {
                sleep(1);
                remaining=((struct thread_info*)thread)->t_res[resource_type].count-((struct thread_info*)thread)->t_res[resource_type].allocated;
                if(remaining==0)
                break;
                resource_number=rand()%remaining+1;
                // Before requesting check if thread is running still or not
                if(((struct thread_info*)thread)->running==false)
                break;
                printf("[Thread %d]: Requesting resource %c = %d\n", ((struct thread_info*)thread)->index, resource[resource_type].name, resource_number);
            }
            
            // Make a random pause
            srand(time(0));
            int pause=rand()%MAX_PAUSE;
            printf("[Thread %d]: Paused for %d seconds\n", ((struct thread_info*)thread)->index, pause);
            sleep(pause);
        }

        if(((struct thread_info*)thread)->running==true)
        {
            printf("[Thread %d]: Acquired all the resources\n\n", ((struct thread_info*)thread)->index);
            // Random pause 0.7d to 1.5d
            int pause=(int)(random_float()*deadlock_detection_interval);
            printf("[Thread %d]: Paused for %d seconds\n", ((struct thread_info*)thread)->index, pause);
            sleep(pause);
            
        }

        if(((struct thread_info*)thread)->running==true)
        {
            printf("[Thread %d]: Releasing Resources\n", ((struct thread_info*)thread)->index);
            // Deallocate the resources
            deallocate((struct thread_info*)thread);
            printf("[Thread %d]: Execution completed\n\n", ((struct thread_info*)thread)->index);
        }

        ((struct thread_info*)thread)->running=false;
    }
    pthread_exit(NULL);
}

// Function to initialize array and matrix used in deadlock detection algorithm
// @params: available, deadlocked, thread_count, thread_need, thread_allocated matrix
// return value: void
void initialize(int available[n], bool deadlocked[max_threads], int thread_count[max_threads][n], int thread_need[max_threads][n], int thread_allocated[max_threads][n])
{
    // Assign available instances for each resource 
    for(int i=0; i<n; i++)
    available[i]=resource[i].count-resource[i].allocated;

    // Intiialized all threads as deadlocked
    for(int i=0; i<max_threads; i++)
    deadlocked[i]=true;
    
    // If thread is not running, make deadlocked=false
    for(int i=0; i<max_threads; i++)
    {
        if(!thread[i].running)
        deadlocked[i]=false;
    }

    // Initialize thread count and allocated and need matrix
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        {
            thread_count[i][j]=thread[i].t_res[j].count;
            thread_allocated[i][j]=thread[i].t_res[j].allocated;
            thread_need[i][j]=thread[i].t_res[j].count-thread[i].t_res[j].allocated;
        }
    }
}

// Function to get the index of a thread which can be executed completely with current available set of resources
// @params: available, deadlocked, thread_count, thread_need, thread_allocated matrix
// return value: index of thread which can be executed
int get_runnable_thread_index(bool deadlocked[max_threads], int thread_need[max_threads][n], int available[n])
{
    int i;
    bool found=true;
    for(i=0; i<max_threads; i++)
    {
        found=true;
        // If thread has already executed or not running
        if(!deadlocked[i]) 
        continue;
       
        for(int j=0; j<n; j++)
        {
            if(thread_need[i][j] > available[j])
            {
                found=false;
                break;
            }
        }
        if(found==true)
        break;
    }
    if(!found)
    i=-1;

    return i;
}

// Function to terminate a particular thread by releasing it's resources 
// @params: index of thread which is to be terminated
// return value: void
void terminate_thread(int i)
{
    printf("Terminating Thread %d\n", i);
    thread[i].running=false;
    for(int j=0; j<n; j++)
    {
        thread[i].t_res[j].count=0;
        resource[j].allocated-=thread[i].t_res[j].allocated;
        thread[i].t_res[j].allocated=0;
    }
    print_current_resource_status();
}

// Function to get the index of a thread which has maximum allocated resource
// @params: thread_allocated matrix, deadlocked array
// return value: index of thread
int max_allocated_resource_thread(int thread_allocated[max_threads][n], bool deadlocked[max_threads])
{
    int max_index, max_resource;
    bool first=true;
    for(int i=0; i<max_threads; i++)
    {
        if(deadlocked[i])
        {
            int resource_allocated=0;
            for(int j=0; j<n; j++)
            {
                resource_allocated+=thread_allocated[i][j];
            }
            if(first)
            {
                max_index=i;
                max_resource=resource_allocated;
                first=false;
                continue;
            }
            if(resource_allocated>max_resource)
            {
                max_index=i;
                max_resource=resource_allocated;
            }
        }
    }
    return max_index;
}

// Function to get the index of a thread which has minimum allocated resource
// @params: thread_allocated matrix, deadlocked array
// return value: index of thread 
int min_allocated_resource_thread(int thread_allocated[max_threads][n], bool deadlocked[max_threads])
{
    int min_index, min_resource;
    bool first=true;
    for(int i=0; i<max_threads; i++)
    {
        if(deadlocked[i])
        {
            int resource_allocated=0;
            for(int j=0; j<n; j++)
            {
                resource_allocated+=thread_allocated[i][j];
            }
            if(first)
            {
                min_index=i;
                min_resource=resource_allocated;
                first=false;
                continue;
            }
            if(resource_allocated<min_resource)
            {
                min_index=i;
                min_resource=resource_allocated;
            }
        }
    }
    return min_index;
}

// Function to get the index of a thread which has maximum allocated instances of a resource type whch is needed most by all threads combined
// @params: thread_allocated matrix, thread_need matrix, deadlocked array
// return value: index of thread
int max_need_max_alloc_thread(int thread_allocated[max_threads][n], int thread_need[max_threads][n], bool deadlocked[max_threads])
{
    // Find resource whose need is maximum
    int resource_type, max_res;
    bool first=true;
    for(int i=0; i<n; i++)
    {
        int sum=0;
        for(int j=0; j<max_threads; j++)
        {
            if(deadlocked[j])
            sum+=thread_need[j][i];
        }
        if(first)
        {
            max_res=sum;
            resource_type=i;
            first=false;
            continue;
        }
        if(sum>max_res)
        {
            max_res=sum;
            resource_type=i;
        }
    }
    // Find thread which holds maximum instances of that resource type
    int index;
    first=true;
    for(int i=0; i<max_threads; i++)
    {
        if(deadlocked[i])
        {
            if(first)
            {
                first=false;
                index=i;
                max_res=thread_allocated[i][resource_type];
                continue;
            }
            if(max_res<thread_allocated[i][resource_type])
            {
                max_res=thread_allocated[i][resource_type];
                index=i;
            }
        }
    }
    return index;
}

// Start routine of thread which is used to detect deadlock
// @params: void
// return value: void
void *deadlock_detector()
{
    int count=0;
    while(1)
    {
        // Periodically check for deadlocks after every interval
        sleep(deadlock_detection_interval);
        // Acquire the lock
        pthread_mutex_lock(&mutex);
        bool deadlock_found=false;
        printf("\n\t\t\t**************DETECTING DEADLOCK**************\n\n");
        count++;
        print_current_resource_status();
        
        int available[n]; // Available number of resources of each type
        bool deadlocked[max_threads]; // True for threads which are deadlocked
        int thread_count[max_threads][n]; // Initial max resource requested by thread
        int thread_need[max_threads][n]; // Resources needed by thread to complete request
        int thread_allocated[max_threads][n]; // Resources allocated to thread

        // Intitialize the array and matrix
        initialize(available, deadlocked, thread_count, thread_need, thread_allocated);
        
        // Deadlock Detection Algorithm
        while(1)
        {
            // Find index of thread which can run to completion with current available resource
            int index=get_runnable_thread_index(deadlocked, thread_need, available);
            
            // No index found 
            if(index==-1)
            {
                printf("\t\t\t\t*Deadlock Detected*\n\n");
                break;
            }  
            else if(index==max_threads)
            {
                break;
            }

            // Update available array
            for(int i=0; i<n; i++)
            {
                available[i]+=thread_allocated[index][i];
            }
            // Mark it as deadlock free
            deadlocked[index]=false;
        }

        for(int i=0; i<max_threads; i++)
        {
            if(deadlocked[i])
            {
                deadlock_found=true;
                printf("\t\t\t\tThread %d deadlocked!!!!!\n", i);
            }
        }
        if(!deadlock_found)
        {
            d[count-1]=false;
            printf("\t\t\t\t*No Deadlock Detected*\n\n");
        }
        else
        {
            // Terminate threads according to heuristic
            if(heuristic==1) // Terminate thread with maximum allocated resources
            {
                printf("\nHeuristic 1: Terminate thread with maximum allocated resources\n");
                int t_index=max_allocated_resource_thread(thread_allocated, deadlocked);
                terminate_thread(t_index);
            }
            else if(heuristic==2) // Terminate thread with minimuum allocated resources
            {
                printf("\nHeuristic 2: Terminate thread with minimum allocated resources\n");
                int t_index=min_allocated_resource_thread(thread_allocated, deadlocked);
                terminate_thread(t_index);
            }
            else if(heuristic==3) // Terminate thread which holds maximum instances of that resource which is needed maximum by all threads combined
            {
                printf("\nHeuristic 3: Terminate thread which holds maximum instances of that resource which is needed maximum by all threads combined\n");
                int t_index=max_need_max_alloc_thread(thread_allocated, thread_need, deadlocked);
                terminate_thread(t_index);
            }
            else if(heuristic==4) // Terminate first thread which is deadlocked
            {
                printf("\nHeuristic 4: Terminate first thread which is deadlocked\n");
                for(int i=0; i<max_threads; i++)
                {
                    if(deadlocked[i])
                    {
                        terminate_thread(i);
                        break;
                    }
                }
            }
            else if(heuristic==5) // Terminate all deadlocked threads
            {
                printf("\nHeuristic 5: Terminate all deadlocked threads\n");
                for(int i=0; i<max_threads; i++)
                {
                    if(deadlocked[i])
                    {
                        terminate_thread(i);
                    }
                }
            } 
            d[count-1]=true;
            gettimeofday(&deadlock_time[count], NULL);
        }        
        // if count exceed max_count, find average time between occurence of deadlock print it and exit the program
        if(count==max_count)
        {
            int d_count=0, prev=0;
            double average=0;
            // Find average time between deadlock
            for(int i=0; i<max_count; i++)
            {
                if(d[i])
                {
                    long seconds = deadlock_time[i+1].tv_sec - deadlock_time[prev].tv_sec;
                    long microseconds = deadlock_time[i+1].tv_usec - deadlock_time[prev].tv_usec;
                    double elapsed = seconds + microseconds*1e-6;
                    printf("Deadlocked occured at time = %.6f seconds\n", elapsed);
                    average+=elapsed;
                    prev=i+1;
                    d_count++;
                }
            }
            if(d_count!=0)
            average/=d_count;

            printf("\nAverage Time elapsed for Heuristic %d = %.6f seconds\n", heuristic, average);

            printf("EXITING THE PROGRAM\n");
            exit(0);
        }
        printf("\n\t\t\t**************END DETECTING DEADLOCK**************\n\n");
        fflush(stdin);
        // Release Lock
        pthread_mutex_unlock(&mutex);
    }
}
 
// CLI Arguments Format
// ./[program_name] [number_of_resources] [name_of resources] [count_of_each_resource] [max_threads] [deadlock_detection_interval] [heuristic_type] [number_of_times_detect_deadlock]
int main(int argc, char*argv[])
{
    srand(time(0));
    if(argc<7)
    {
        printf("[*]Info: CLI Arguments should be of the form: ./[program_name] [number_of_resources] [name_of resources] [count_of_each_resource] [max_threads] [deadlock_detection_interval] [heuristic_type] [number_of_times_detect_deadlock]\n");
        exit(0);
    }
    printf("[+]Success: Main program starting\n\n");
    
    // Extract values from the CLI argument
    n = atoi(argv[1]); // Number of type of resources
    resource = (struct resource_info*)malloc(n*sizeof(struct resource_info));
    char name[n];
    for(int i=0; i<n; i++)
    {
        name[i]=argv[2+i][0];
        resource[i].name=argv[2+i][0];
    }
    for(int i=0; i<n; i++)
    {
        resource[i].count=atoi(argv[2+n+i]);
        resource[i].allocated=0; // Initially allocated resources = 0 i.e. all are free 
    }
    max_threads = atoi(argv[2+2*n]);
    deadlock_detection_interval = atoi(argv[3+2*n]);
    heuristic = atoi(argv[4+2*n]);
    max_count = atoi(argv[5+2*n]);

    deadlock_time = (struct timeval*)malloc((max_count+1)*sizeof(struct timeval));
    d = (bool*)malloc(max_count*sizeof(bool));
    gettimeofday(&deadlock_time[0], NULL);

    // Create a thread pool and intitialise it
    thread = (struct thread_info*)malloc(max_threads*sizeof(struct thread_info));
    for(int i=0; i<max_threads; i++)
    {
        thread[i].index=i;
        thread[i].running=false; // State of each thread is not running intiially
        thread[i].t_res=(struct resource_info*)malloc(n*sizeof(struct resource_info)); // Struct to store resource information for each thread
    }
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        {
            thread[i].t_res[j].name=resource[j].name;
            thread[i].t_res[j].count=0; // Resources needed by thread 
            thread[i].t_res[j].allocated=0; // Resources allocated to thread
        }
    }
    print_current_resource_status();
    
    // Create Thread and start executing each thread
    for(int i=0; i<max_threads; i++)
    {
        pthread_create(&thread[i].t_id, NULL, execute, (void*)&thread[i]);
    }
    // Create a separate thread for deadlock detection
    pthread_t detector;
    pthread_create(&detector, NULL, deadlock_detector, NULL);
    
    pthread_exit(NULL);
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
    return 0;
}
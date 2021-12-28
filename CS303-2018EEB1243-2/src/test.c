#include<stdio.h>
#include<stdlib.h> // for malloc(), rand()
#include<unistd.h> // for sleep
#include<stdbool.h> // for bool
#include<pthread.h> // for pthread APIs

int n=3; // Test value of number of resource
int max_threads=4; // Test value for number of max threads

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

// Function to check whether current thread needs more resource or not
// @params: pointer to thread_info struct for that thread
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

// Function to genrate a random floating point number between [0.7, 1.5]
// @params: void
// return value: floating point number in [0.7, 1.5]
float random_float()
{
    return 0.7+((float)rand()/RAND_MAX)*(1.5-0.7);
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

void test_need_resource()
{
    printf("*********************Testing need_resource()*********************\n\n");
    struct thread_info* t = (struct thread_info*)malloc(sizeof(struct thread_info));
    t->t_res=(struct resource_info*)malloc(sizeof(struct resource_info));

    for(int i=0; i<n; i++)
    {
        t->t_res[i].count=4;
        t->t_res[i].allocated=4;
    }
    if(need_resource(t)==false)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    t->t_res[1].allocated=0;
    if(need_resource(t)==true)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
}

void test_random_float()
{
    printf("*********************Testing random_float()*********************\n\n");
    if(random_float()>=0.7 && random_float()<=1.5)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_get_runnable_thread_index()
{
    printf("*********************Testing get_runnable_thread_index()*********************\n\n");
    bool deadlocked[max_threads];
    for(int i=0; i<max_threads; i++)
    deadlocked[i]=true;
    int thread_need[max_threads][n];
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        thread_need[i][j]=4;
    }
    int available[n];
    for(int i=0; i<n; i++)
    available[i]=5;

    if(get_runnable_thread_index(deadlocked, thread_need, available)==0)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    available[0]=3;
    if(get_runnable_thread_index(deadlocked, thread_need, available)==-1)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
}

void test_max_allocated_resource_thread()
{
    printf("*********************Testing max_allocated_resource_thread()*********************\n\n");
    int thread_allocated[max_threads][n];
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        thread_allocated[i][j]=4;
    }

    bool deadlocked[max_threads];
    for(int i=0; i<max_threads; i++)
    deadlocked[i]=true;

    thread_allocated[2][1]=10;
    if(max_allocated_resource_thread(thread_allocated, deadlocked)==2)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }

    thread_allocated[3][0]=20;
    if(max_allocated_resource_thread(thread_allocated, deadlocked)==3)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
}

void test_min_allocated_resource_thread()
{
    printf("*********************Testing min_allocated_resource_thread()*********************\n\n");
    int thread_allocated[max_threads][n];
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        thread_allocated[i][j]=4;
    }
    bool deadlocked[max_threads];
    for(int i=0; i<max_threads; i++)
    deadlocked[i]=true;

    thread_allocated[2][1]=2;
    if(min_allocated_resource_thread(thread_allocated, deadlocked)==2)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }

    thread_allocated[3][0]=1;
    if(min_allocated_resource_thread(thread_allocated, deadlocked)==3)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
}

void test_max_need_max_alloc_thread()
{
    printf("*********************Testing max_need_max_alloc_thread()*********************\n\n");
    int thread_allocated[max_threads][n], thread_need[max_threads][n];
    for(int i=0; i<max_threads; i++)
    {
        for(int j=0; j<n; j++)
        {
            if(i==1)
            thread_allocated[i][j]=10;
            else
            thread_allocated[i][j]=4;
        }
    }
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<max_threads; j++)
        {
            if(i==0)
            thread_need[j][i]=5;
            else
            thread_need[j][i]=4;
        }
    }
    bool deadlocked[max_threads];
    for(int i=0; i<max_threads; i++)
    deadlocked[i]=true;

    if(max_need_max_alloc_thread(thread_allocated, thread_need, deadlocked)==1)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}
int main()
{
    test_need_resource();
    test_random_float();
    test_get_runnable_thread_index();
    test_max_allocated_resource_thread();
    test_min_allocated_resource_thread();
    test_max_need_max_alloc_thread();
    return 0;
}
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>

// Struct to store process info
struct process_info{
    int arrival_time;
    int duration;
    int size;
    int turnaround_time;
    int allocation_time;
    int end_time;
    bool allocated;
};

// Struct to store info for each algorithm
struct algo_info{
    int index;
    struct process_info* process;
    int *memory;
    int previous; // For next fit
    double memory_utilization;
    double average_turnaround;
};

// Global Variables
struct algo_info first_fit, next_fit, best_fit;

// Function to generate random float number in the given range
float random_float(float min, float max)
{
    float num = min+((float)rand()/RAND_MAX)*(max-min);
    return num;
}

// Function to generate a random number between (0.5m, 3m) in multiples of 10
int find_random_size(int m)
{
    int max_size=m*3;
    int p_size = 10*(rand()%(max_size/10)+1);
    return p_size;
} 

// Function to generate a random number between (0.5t, 6t) in multiples of 5
int find_random_duration(int t)
{
    int start=t*0.5, end=6*t; 
    int min_duration = (start%5==0)? start: ((start/5)+1)*5;
    int max_duration = (end%5==0)? end: (end/5)*5;
    int total = (max_duration-min_duration)/5 + 1;
    int p_duration = min_duration + (rand()%total)*5;
    return p_duration; 
} 

// Function to initialise process info
void initialise(int max_process, int blocks, int m, int t, int r)
{
    first_fit.index=0;
    first_fit.process=(struct process_info*)malloc(max_process*sizeof(struct process_info));
    first_fit.memory=(int*)malloc(blocks*sizeof(int));
    first_fit.memory_utilization=0.0;
    first_fit.previous=0;
    first_fit.average_turnaround=0.0;

    next_fit.index=0;
    next_fit.process=(struct process_info*)malloc(max_process*sizeof(struct process_info));
    next_fit.memory=(int*)malloc(blocks*sizeof(int));
    next_fit.previous=0; 
    next_fit.memory_utilization=0.0;
    next_fit.average_turnaround=0.0;
    
    best_fit.index=0;
    best_fit.process=(struct process_info*)malloc(max_process*sizeof(struct process_info));
    best_fit.memory=(int*)malloc(blocks*sizeof(int));
    best_fit.memory_utilization=0.0;
    best_fit.previous=0;
    best_fit.average_turnaround=0.0;

    for(int i=0; i<max_process; i++)
    {
        int size=find_random_size(m);
        int duration=find_random_duration(t);

        first_fit.process[i].arrival_time=i/r;
        first_fit.process[i].duration=duration;
        first_fit.process[i].size=size;
        first_fit.process[i].turnaround_time=0;
        first_fit.process[i].allocation_time=0;
        first_fit.process[i].end_time=0;
        first_fit.process[i].allocated=false;

        next_fit.process[i].arrival_time=i/r;
        next_fit.process[i].duration=duration;
        next_fit.process[i].size=size;
        next_fit.process[i].turnaround_time=0;
        next_fit.process[i].allocation_time=0;
        next_fit.process[i].end_time=0;
        next_fit.process[i].allocated=false;

        best_fit.process[i].arrival_time=i/r;
        best_fit.process[i].duration=duration;
        best_fit.process[i].size=size;
        best_fit.process[i].turnaround_time=0;
        best_fit.process[i].allocation_time=0;
        best_fit.process[i].end_time=0;
        best_fit.process[i].allocated=false;
    }

    for(int i=0; i<blocks; i++)
    {
        first_fit.memory[i]=-1;
        next_fit.memory[i]=-1;
        best_fit.memory[i]=-1;
    }

}

// Function to deallocate memory
void deallocate(int blocks, int time)
{
    for(int i=0; i<blocks; i++)
    {
        if(first_fit.memory[i]!=-1)
        {
            int id=first_fit.memory[i];
            int end_time=first_fit.process[id].end_time;
            if(end_time<=time)
            {
                int count=first_fit.process[id].size/10;
                for(int j=i; j<i+count; j++)
                {
                    first_fit.memory[j]=-1;
                }
            } 
        }

        if(next_fit.memory[i]!=-1)
        {
            int id=next_fit.memory[i];
            int end_time=next_fit.process[id].end_time;
            if(end_time<=time)
            {
                int count=next_fit.process[id].size/10;
                for(int j=i; j<i+count; j++)
                {
                    next_fit.memory[j]=-1;
                }
            } 
        }

        if(best_fit.memory[i]!=-1)
        {
            int id=best_fit.memory[i];
            int end_time=best_fit.process[id].end_time;
            if(end_time<=time)
            {
                int count=best_fit.process[id].size/10;
                for(int j=i; j<i+count; j++)
                {
                    best_fit.memory[j]=-1;
                }
            } 
        }
    }
}

// Function to find index according to BEST FIT ALGORITHM
int find_best_fit(int blocks, int required)
{
    int min_mem=INT_MAX, min_idx=-1;
    for(int i=0; i<=blocks-required; i++)
    {
        if(best_fit.memory[i]==-1)
        {
            int count=0, j;
            for(j=i; j<blocks; j++)
            {
                if(best_fit.memory[j]==-1)
                count++;
                else break;
            }
            if(count>=required)
            {
                if(count<min_mem)
                {
                    min_mem=count;
                    min_idx=i;
                }
            }
            i=j-1;
        }  
    }
    return min_idx;
}

// Function to find index according to FIRST FIT ALGORITHM
int find_first_fit(int blocks, int required)
{
    int idx=-1;
    for(int i=0; i<=blocks-required; i++)
    {
        if(first_fit.memory[i]==-1)
        {
            int count=0;
            for(int j=i; j<i+required; j++)
            {
                if(first_fit.memory[j]==-1)
                count++;
            }
            if(count==required)
            {
                idx=i;
                break;
            }
        }
    } 
    return idx;
}

// Function to find index according to NEXT FIT ALGORITHM
int find_next_fit(int required, int blocks)
{
    int idx=-1;
    int index=next_fit.previous;
    for(int i=0; i<blocks; i++)
    {
        if(index>blocks-required)
        {
            index++;
            if(index==blocks)index=0;
            continue;
        }
        if(next_fit.memory[index]==-1)
        {
            int count=0;
            for(int j=index; j<index+required; j++)
            {
                if(next_fit.memory[j]==-1)
                count++;
            }
            if(count==required)
            {
                idx=index;
                break;
            }
        } 
        index++;
        if(index==blocks)index=0;
    }
    return idx;
}

// Function to find memory utilization
void find_memory_utilization(int blocks, int p, int q)
{
    int memory_used=q;
    for(int i=0; i<blocks; i++)
    {
        if(first_fit.memory[i]!=-1)
        memory_used+=10;
    }
    first_fit.memory_utilization=(double)memory_used/p;
    
    memory_used=q;
    for(int i=0; i<blocks; i++)
    {
        if(next_fit.memory[i]!=-1)
        memory_used+=10;
    }
    next_fit.memory_utilization=(double)memory_used/p;

    memory_used=q;
    for(int i=0; i<blocks; i++)
    {
        if(best_fit.memory[i]!=-1)
        memory_used+=10;
    }
    best_fit.memory_utilization=(double)memory_used/p;
}

// Function to find average turnaround time
void find_average_turnaround(int max_process)
{
    int turnaround=0, count=0;
    for(int i=0; i<max_process; i++)
    {
        if(first_fit.process[i].allocated)
        {
            turnaround+=first_fit.process[i].turnaround_time;
            count++;
        }
    }
    if(count!=0)
    first_fit.average_turnaround=(double)turnaround/count;

    turnaround=0;
    count=0;
    for(int i=0; i<max_process; i++)
    {
        if(next_fit.process[i].allocated)
        {
            turnaround+=next_fit.process[i].turnaround_time;
            count++;
        }
    }
    if(count!=0)
    next_fit.average_turnaround=(double)turnaround/count;

    turnaround=0;
    count=0;
    for(int i=0; i<max_process; i++)
    {
        if(best_fit.process[i].allocated)
        {
            turnaround+=best_fit.process[i].turnaround_time;
            count++;
        }
    }
    if(count!=0)
    best_fit.average_turnaround=(double)turnaround/count;
}

void print_first_fit(int blocks)
{
    printf("FIRST FIT MEMORY->");
    for(int i=0; i<blocks; i++)
    printf("%d ", first_fit.memory[i]);
    printf("\n");
}

void print_next_fit(int blocks)
{
    printf("NEXT FIT MEMORY->");
    for(int i=0; i<blocks; i++)
    printf("%d ", next_fit.memory[i]);
    printf("\n");
}

void print_best_fit(int blocks)
{
    printf("BEST FIT MEMORY->");
    for(int i=0; i<blocks; i++)
    printf("%d ", best_fit.memory[i]);
    printf("\n");
}

void print_process_info_first_fit(int max_process)
{
    printf("Process Id\tArrival Time\tSize\tDuration\tEnd Time\tTurnaround\tAllocation\n");
    for(int i=0; i<max_process; i++)
    {
        if(first_fit.process[i].allocated)
        {
            printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", i, first_fit.process[i].arrival_time, first_fit.process[i].size, first_fit.process[i].duration, first_fit.process[i].end_time, first_fit.process[i].turnaround_time, first_fit.process[i].allocation_time);
        }
    }
}

void print_process_info_next_fit(int max_process)
{
    printf("Process Id\tArrival Time\tSize\tDuration\tTurnaround\tAllocation\tFirstAlloc\tTurnaround\n");
    for(int i=0; i<max_process; i++)
    {
        if(next_fit.process[i].allocated)
        {
            printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", i, next_fit.process[i].arrival_time, next_fit.process[i].size, next_fit.process[i].duration, next_fit.process[i].turnaround_time, next_fit.process[i].allocation_time,  first_fit.process[i].allocation_time, first_fit.process[i].turnaround_time);
        }
    }
}

void print_process_info_best_fit(int max_process)
{
    printf("Process Id\tArrival Time\tSize\tDuration\tEnd Time\tTurnaround\tAllocation\n");
    for(int i=0; i<max_process; i++)
    {
        if(best_fit.process[i].allocated)
        {
            printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", i, best_fit.process[i].arrival_time, best_fit.process[i].size, best_fit.process[i].duration, best_fit.process[i].end_time, best_fit.process[i].turnaround_time, best_fit.process[i].allocation_time);
        }
    }
}

void print_statistics()
{
    printf("\n\t\t\tFIRST_FIT\t\tNEXT_FIT\t\tBEST_FIT\n");
    
    printf("Memory Utilization \t%f\t\t%f\t\t%f\n", first_fit.memory_utilization*100.0, next_fit.memory_utilization*100.0, best_fit.memory_utilization*100.0);
    printf("Average Turnaround \t%f\t\t%f\t\t%f\n\n", first_fit.average_turnaround, next_fit.average_turnaround, best_fit.average_turnaround);
}

int main(int argc, char*argv[])
{
    srand(time(0));
    if(argc < 7)
    {
        printf("Info: CLI Arguments should be of the type ./[program_name] [p] [q] [n] [m] [t] [T]\n");
        exit(0);
    }

    // Extract arguments
    int p = atoi(argv[1]);
    int q = atoi(argv[2]);
    int n = atoi(argv[3]);
    int m = atoi(argv[4]);
    int t = atoi(argv[5]);
    int total_time = atoi(argv[6])*60; // Store in seconds
    int r = (int)random_float(0.1*n, 1.2*n); 
    // Maximum process that can arrive
    int max_process = total_time*r;
    // Actual Physical Memory available for allocation
    int memory_available = p-q;
    // Assuming block size is 10MB each index represents 10MB as minimum size process is 10 MB
    int blocks = memory_available/10;
    initialise(max_process, blocks, m, t, r);

    for(int time=0; time<total_time; time++)
    {
        // Deallocate Memory
        deallocate(blocks, time);

        // Try Allocating First Fit
        for(int idx=first_fit.index; idx<max_process; idx++)
        {
            bool success=false;
            
            if(time>=first_fit.process[first_fit.index].arrival_time)
            {
                int required=first_fit.process[first_fit.index].size/10;
                success=false;
                int first_idx=find_first_fit(blocks, required);
                if(first_idx!=-1)
                {
                    for(int j=first_idx; j<first_idx+required; j++)
                    first_fit.memory[j]=first_fit.index;

                    first_fit.process[first_fit.index].allocated=true;
                    first_fit.process[first_fit.index].end_time=first_fit.process[first_fit.index].duration+time;
                    first_fit.process[first_fit.index].turnaround_time=time-first_fit.process[first_fit.index].arrival_time;
                    first_fit.process[first_fit.index].allocation_time=time;
                    first_fit.index++;
                    success=true;
                }
            }
            if(!success)
            break;
        } 
        
        // Try allocating Next Fit
        for(int idx=next_fit.index; idx<max_process; idx++)
        {
            bool success=false;
            if(time>=next_fit.process[next_fit.index].arrival_time)
            {
                int required=next_fit.process[next_fit.index].size/10;
                success=false;

                int next_idx=find_next_fit(required, blocks);
                
                if(next_idx!=-1)
                {
                    for(int j=next_idx; j<next_idx+required; j++)
                    next_fit.memory[j]=next_fit.index;

                    next_fit.process[next_fit.index].allocated=true;
                    next_fit.process[next_fit.index].end_time=next_fit.process[next_fit.index].duration+time;
                    next_fit.process[next_fit.index].turnaround_time=time-next_fit.process[next_fit.index].arrival_time;
                    next_fit.process[next_fit.index].allocation_time=time;
                    next_fit.previous=next_idx+required;
                    if(next_fit.previous==blocks)next_fit.previous=0;
                    next_fit.index++;
                    success=true;
                }
            }
            if(!success)
            break;
        } 

        // Try allocating Best Fit
        for(int idx=best_fit.index; idx<max_process; idx++)
        {
            bool success=false;
            if(time>=best_fit.process[best_fit.index].arrival_time)
            {
                int required=best_fit.process[best_fit.index].size/10;
                int min_idx=find_best_fit(blocks, required);
                success=false;
                if(min_idx!=-1)
                {
                    // Allocate memory
                    for(int j=min_idx; j<min_idx+required; j++)
                    best_fit.memory[j]=best_fit.index;

                    best_fit.process[best_fit.index].allocated=true;
                    best_fit.process[best_fit.index].end_time=best_fit.process[best_fit.index].duration+time;
                    best_fit.process[best_fit.index].turnaround_time=time-best_fit.process[best_fit.index].arrival_time;
                    best_fit.process[best_fit.index].allocation_time=time;
                    best_fit.index++;
                    success=true;
                }  
            }
            if(!success)
            break;
        } 
    }

    find_memory_utilization(blocks, p, q);
    find_average_turnaround(max_process);
    print_statistics();

    free(first_fit.process);
    free(first_fit.memory);
    free(next_fit.process);
    free(next_fit.memory);
    free(best_fit.process);
    free(best_fit.memory);

    return 0;
}
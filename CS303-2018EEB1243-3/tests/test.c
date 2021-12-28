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

// Function to initialise process info
void initialise(int max_process, int blocks, int r)
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
        int size=10;
        int duration=5;

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

void test_find_first_fit()
{
    int max_process=4, blocks=10, r=2, required=3;
    initialise(max_process, blocks, r);
    printf("*********************Testing find_first_fit()*********************\n\n");
    for(int i=0; i<blocks; i++)
    {
        if(i==0||i==4||i==5||i==6)
        first_fit.memory[i]=1;
    }
    int index=find_first_fit(blocks, required);
    if(index==1)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    for(int i=0; i<blocks; i++)
    {
        if(i==1||i==2||i==3)
        first_fit.memory[i]=1;
    }
    index=find_first_fit(blocks, required);
    if(index==7)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
    first_fit.memory[7]=1;
    index=find_first_fit(blocks, required);
    if(index==-1)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }
}

void test_find_next_fit()
{
    int max_process=4, blocks=10, r=2, required=3;
    initialise(max_process, blocks, r);
    printf("*********************Testing find_next_fit()*********************\n\n");
    for(int i=0; i<blocks; i++)
    {
        if(i==0||i==4||i==5||i==6)
        next_fit.memory[i]=1;
    }
    next_fit.previous=7;
    int index=find_next_fit(required, blocks);
    if(index==7)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    next_fit.previous=0;
    index=find_next_fit(required, blocks);
    if(index==1)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }

    for(int i=0; i<blocks; i++)
    {
        if(i==1||i==7)
        next_fit.memory[i]=1;
    }
    index=find_next_fit(required, blocks);
    if(index==-1)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }
}

void test_find_best_fit()
{
    int max_process=4, blocks=10, r=2, required=3;
    initialise(max_process, blocks, r);
    printf("*********************Testing find_best_fit()*********************\n\n");
    for(int i=0; i<blocks; i++)
    {
        if(i==5)
        best_fit.memory[i]=1;
    }
    int index=find_best_fit(blocks, required);
    if(index==6)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    for(int i=0; i<blocks; i++)
    {
        if(i==0||i==1||i==9)
        best_fit.memory[i]=1;
    }
    index=find_best_fit(blocks, required);
    if(index==2)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
    for(int i=0; i<blocks; i++)
    {
        best_fit.memory[i]=1;
    }
    index=find_best_fit(blocks, required);
    if(index==-1)
    {
        printf("[+]Test Case 3: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 3: Failed\n\n");
    }
}

void test_find_random_duration()
{
    printf("*********************Testing find_random_duration()*********************\n\n");
    int num=find_random_duration(20);
    if(num>=0.5*20 && num<=6*20 && num%5==0)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_random_size()
{
    printf("*********************Testing find_random_size()*********************\n\n");
    int num=find_random_size(20);
    if(num>=0.5*20 && num<=3*20 && num%10==0)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_random_float()
{
    printf("*********************Testing random_float()*********************\n\n");
    float num = random_float(1.2, 4.7);
    if(num>=1.2 && num<=4.7)
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
    test_find_first_fit();
    test_find_next_fit();
    test_find_best_fit();
    test_find_random_duration();
    test_find_random_size();
    test_random_float();
}
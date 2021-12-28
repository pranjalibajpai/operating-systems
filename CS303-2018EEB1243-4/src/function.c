#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include <time.h>
#include<limits.h>
#include<math.h>

#define MAX_REQUEST 10 // Can be changed as per need. Make sure to change it in main.c and test.c as well. Same goes for MAX_PLATTER MAX_CYLINDER & MAX_SECTOR
#define MAX_PLATTER 4
#define MAX_CYLINDER 25
#define MAX_SECTOR 20

// Structure to store request information
struct request_info
{ 
    int id;
    int cylinder; // Cylinder number
    int sector; // Sector number
    int number; // Number of sectors requested
};

// Structure to store information about each scheduling policy
struct scheduling_policy
{
    double throughput;
    double response_time[MAX_REQUEST];
};

// Function to initialize Read/Write head position
void initialize_readwrite_head(int rw_head[MAX_PLATTER])
{
    for(int i=0; i<MAX_PLATTER; i++)
    rw_head[i]=0;
}

// Function to initialize request info
void initialize_request(struct request_info request[MAX_PLATTER][MAX_REQUEST])
{
    for(int i=0; i<MAX_PLATTER; i++)
    {
        for(int j=0; j<MAX_REQUEST; j++)
        {
            request[i][j].id=-1;
            request[i][j].cylinder=-1;
            request[i][j].sector=-1;
            request[i][j].number=-1;
        }
    }
}

// Function to initilaize scheduling policy information
void initialize_scheduling_policy(struct scheduling_policy *random, struct scheduling_policy *fifo, struct scheduling_policy *sstf, struct scheduling_policy *scan, struct scheduling_policy *cscan)
{
    random->throughput=0.0;
    fifo->throughput=0.0;
    sstf->throughput=0.0;
    scan->throughput=0.0;
    cscan->throughput=0.0;
    for(int i=0; i<MAX_REQUEST; i++) 
    {
        random->response_time[i]=0.0;
        fifo->response_time[i]=0.0;
        sstf->response_time[i]=0.0;
        scan->response_time[i]=0.0;
        cscan->response_time[i]=0.0;
    }
}

// Function to generate set of random requests
void generate_random_request(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_PLATTER])
{
    for(int i=0; i<MAX_REQUEST; i++)
    {
        int platter_no=rand()%4; // Generate platter number randomly
        request[platter_no][count[platter_no]].id=i;
        request[platter_no][count[platter_no]].cylinder=rand()%MAX_CYLINDER;
        request[platter_no][count[platter_no]].sector=rand()%MAX_SECTOR;
        request[platter_no][count[platter_no]].number=rand()%MAX_SECTOR+1;
        count[platter_no]++;
    }
}

// Function to find data transfer time in ms
double data_transfer_time(int number, int r)
{
    double time=(double)(60*1000*number)/(r*MAX_SECTOR);
    return time;
}

// Function to find total response time for a request in ms
double find_response_time(int r, int Ts, int rw_head, int cylinder, int number)
{
    double time=0.0;        
    // Seek time: time to move the head to required cylinder
    time=time+(double)(Ts*(abs(rw_head-cylinder)));
    // Rotational Delay: time to move the head to required sector(half the revolution time)
    // r revolution/minute | 1 revolution => 1/r minutes | 1 revolution => 60*1000/r milliseconds
    time=time+(double)((60*1000)/(2*r));
    // Data Transfer time
    time=time+data_transfer_time(number, r);
    return time;
}

// Function to find random sequence of requests
void find_random_sequence(int random[], int count)
{
    for(int i=0; i<count; i++)
    random[i]=i;

    for(int i=0; i<count; i++)
    {
        int j=rand()%(i+1);
        int temp=random[i];
        random[i]=random[j];
        random[j]=temp;
    }
}

// Function to find shortest service time request
int find_sst_request(int platter, struct request_info request[MAX_REQUEST], struct scheduling_policy *sstf, int count, int rw_head)
{
    int sst=INT_MAX, sst_idx=-1;
    for(int i=0; i<count; i++)
    {
        int id=request[i].id;
        if(sstf->response_time[id]==0.0)
        {
            int st=abs(rw_head-request[i].cylinder);
            if(st<sst)
            {
                sst=st;
                sst_idx=i;
            }
        }
    }
    return sst_idx;
}

// Function to find SCAN sequence of requests
void find_scan_sequence(int scan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST])
{
    int i=0;
    for(i=0; i<count; i++)
    scan_seq[i]=i;
    bool schedule[count];
    for(i=0; i<count; i++)
    schedule[i]=false;

    // RIGHT
    for(i=0; i<count; i++)
    {
        int min=INT_MAX, idx=-1;
        for(int j=0; j<count; j++)
        {
            int cylinder=request[j].cylinder;
            if(cylinder>=rw_head && !schedule[j])
            {
                if(cylinder<min)
                {
                    min=cylinder;
                    idx=j;
                }
            }
        }
        if(idx==-1)
        break;
        scan_seq[i]=idx;
        schedule[idx]=true;
    }
    // LEFT
    for(; i<count; i++)
    {
        int max=INT_MIN, idx=-1;
        for(int j=0; j<count; j++)
        {
            int cylinder=request[j].cylinder;
            if(cylinder<rw_head && !schedule[j])
            {
                if(cylinder>max)
                {
                    max=cylinder;
                    idx=j;
                }
            }
        }
        if(idx==-1)
        break;
        scan_seq[i]=idx;
        schedule[idx]=true;
    }
}

// Function to find CSCAN sequence of requests
void find_cscan_sequence(int cscan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST])
{
    int i=0;
    for(i=0; i<count; i++)
    cscan_seq[i]=i;
    bool schedule[count];
    for(i=0; i<count; i++)
    schedule[i]=false;

    // RIGHT
    for(i=0; i<count; i++)
    {
        int min=INT_MAX, idx=-1;
        for(int j=0; j<count; j++)
        {
            int cylinder=request[j].cylinder;
            if(cylinder>=rw_head && !schedule[j])
            {
                if(cylinder<min)
                {
                    min=cylinder;
                    idx=j;
                }
            }
        }
        if(idx==-1)
        break;
        cscan_seq[i]=idx;
        schedule[idx]=true;
    }
    // RIGHT
    for(; i<count; i++)
    {
        int min=INT_MAX, idx=-1;
        for(int j=0; j<count; j++)
        {
            int cylinder=request[j].cylinder;
            if(cylinder<rw_head && !schedule[j])
            {
                if(cylinder<min)
                {
                    min=cylinder;
                    idx=j;
                }
            }
        }
        if(idx==-1)
        break;
        cscan_seq[i]=idx;
        schedule[idx]=true;
    }
}

// PRINTING UTILITY FUNCTIONS 

// Function to print set of randomly generated requests
void print_request(struct request_info request[MAX_PLATTER][MAX_REQUEST])
{
    printf("ID\tPlatter\t\tCylinder\tSector\tNumber of sectors\n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        for(int j=0; j<MAX_REQUEST; j++)
        {
            if(request[i][j].cylinder == -1)
            break;
            printf("%d\t%d\t\t%d\t\t%d\t\t%d\n", request[i][j].id, i, request[i][j].cylinder, request[i][j].sector, request[i][j].number);
        }      
    }
}

// Function to print repsonse time of a scheduling policy
void print_result(struct scheduling_policy *algo)
{
    double min=INT_MAX, max=INT_MIN, sum=0.0, avg=0.0, stddev=0.0;
    for(int i=0; i<MAX_REQUEST; i++)
    {
        double time=algo->response_time[i];
        printf("%.6f ", time);
        if(time<min)
        min=time;
        if(time>max)
        max=time;
        sum=sum+time;
    }
    if(sum!=0.0)
    avg=sum/MAX_REQUEST;
    for(int i=0; i<MAX_REQUEST; i++)
    {
        stddev+=pow(algo->response_time[i]-avg, 2);
    }
    stddev/=MAX_REQUEST;
    stddev=sqrt(stddev);

    printf("\nMAX\t\tMIN\t\tAVG\t\tSTDDEV\t\tTHROUGHPUT\n");
    printf("%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n", max, min, avg, stddev, algo->throughput);
    printf("\n");
}
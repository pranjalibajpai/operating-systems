#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include <time.h>
#include<limits.h>
#include<math.h>
#include"function.h"

#define MAX_REQUEST 10 // Can be changed as per need. Make sure to change it in function.c and test.c as well. Same goes for MAX_PLATTER MAX_CYLINDER & MAX_SECTOR
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

void initialize_readwrite_head(int rw_head[MAX_PLATTER]);
void initialize_request(struct request_info request[MAX_PLATTER][MAX_REQUEST]);
void initialize_scheduling_policy(struct scheduling_policy *random, struct scheduling_policy *fifo, struct scheduling_policy *sstf, struct scheduling_policy *scan, struct scheduling_policy *cscan);
void print_request(struct request_info request[MAX_PLATTER][MAX_REQUEST]);
void print_result(struct scheduling_policy *algo);
void generate_random_request(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_PLATTER]);
int find_sst_request(int platter, struct request_info request[MAX_REQUEST], struct scheduling_policy *sstf, int count, int rw_head);
void find_random_sequence(int random[], int count);
void find_scan_sequence(int scan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST]);
void find_cscan_sequence(int cscan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST]);

// Function to perform RANDOM Scheduling of set of requests
double random_scheduler(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_REQUEST], struct scheduling_policy *random, int rw_head[MAX_PLATTER], int r, int n, int Ts)
{
    double max_response_time=0.0, sum=0.0;
    printf("\nRANDOM ORDER: \n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        int prev=-1;
        int random_seq[count[i]];
        sum=0.0;
        find_random_sequence(random_seq, count[i]);
        printf("PLATTER %d: ", i);
        for(int j=0; j<count[i]; j++)
        {
            int idx=random_seq[j];
            int id=request[i][idx].id;
            printf("%d ", id);
            random->response_time[id]=find_response_time(r, Ts, rw_head[i], request[i][idx].cylinder, request[i][idx].number);
            prev=id;
            rw_head[i]=request[i][idx].cylinder; // Update RW Head
            sum+=random->response_time[id];
        }
        if(sum>max_response_time)
        max_response_time=sum;
    }
    printf("\n");
    max_response_time/=1000; // ms to seconds
    return (double)MAX_REQUEST/max_response_time;
}

// Function to perform FIFO Scheduling of set of requests
double fifo_scheduler(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_REQUEST], struct scheduling_policy *fifo, int rw_head[MAX_PLATTER], int r, int n, int Ts)
{
    double max_response_time=0.0, sum=0.0;
    printf("\nFIFO ORDER: \n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        int prev=-1;
        sum=0.0;
        printf("PLATTER %d: ", i);
        for(int j=0; j<count[i]; j++)
        {
            int id=request[i][j].id;
            
            fifo->response_time[id]=find_response_time(r, Ts, rw_head[i], request[i][j].cylinder, request[i][j].number);
            printf("%d ", id);

            prev=id;
            rw_head[i]=request[i][j].cylinder; // Update the RW head position
            sum+=fifo->response_time[id];
        }
        if(sum>max_response_time)
        max_response_time=sum;
    }
    printf("\n");
    max_response_time/=1000; // ms to seconds
    return (double)MAX_REQUEST/max_response_time;
}

// Function to perform SSTF Scheduling of set of requests
double sstf_scheduler(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_REQUEST], struct scheduling_policy *sstf, int rw_head[MAX_PLATTER], int r, int n, int Ts)
{
    double max_response_time=0.0, sum=0.0;
    printf("\nSSTF ORDER: \n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        int prev=-1;
        sum=0.0;
        printf("PLATTER %d: ", i);
        for(int j=0; j<count[i]; j++)
        {
            int idx=find_sst_request(i, request[i], sstf, count[i], rw_head[i]);
            int id=request[i][idx].id;
            printf("%d ", id);
            sstf->response_time[id]=find_response_time(r, Ts, rw_head[i], request[i][idx].cylinder, request[i][idx].number);
            prev=id;
            rw_head[i]=request[i][idx].cylinder; // Update RW head
            sum+=sstf->response_time[id];
        }
        if(sum>max_response_time)
        max_response_time=sum;
    }
    printf("\n");
    max_response_time/=1000; // ms to seconds
    return (double)MAX_REQUEST/max_response_time;
}

// Function to perform SCAN Scheduling of set of requests
double scan_scheduler(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_REQUEST], struct scheduling_policy *scan, int rw_head[MAX_PLATTER], int r, int n, int Ts)
{
    double max_response_time=0.0, sum=0.0;
    printf("\nSCAN ORDER: \n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        rw_head[i]=rand()%MAX_CYLINDER;
        int prev=-1;
        sum=0.0;
        int scan_seq[count[i]];
        find_scan_sequence(scan_seq, count[i], rw_head[i], request[i]);
        printf("\nRW HEAD: %d PLATTER %d: ", rw_head[i], i);
        for(int j=0; j<count[i]; j++)
        {
            int idx=scan_seq[j];
            int id=request[i][idx].id;
            printf("%d ", id);
            scan->response_time[id]=find_response_time(r, Ts, rw_head[i], request[i][idx].cylinder, request[i][idx].number);
            prev=id;
            rw_head[i]=request[i][idx].cylinder; // Update the RW head
            sum+=scan->response_time[id];
        }
        if(sum>max_response_time)
        max_response_time=sum;
    }
    printf("\n");
    max_response_time/=1000; // ms to seconds
    return (double)MAX_REQUEST/max_response_time;
}

// Function to perform CSCAN Scheduling of set of requests
double cscan_scheduler(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_REQUEST], struct scheduling_policy *cscan, int rw_head[MAX_PLATTER], int r, int n, int Ts)
{
    double max_response_time=0.0, sum=0.0;
    printf("\nCSCAN ORDER: \n");
    for(int i=0; i<MAX_PLATTER; i++)
    {
        rw_head[i]=rand()%MAX_CYLINDER;
        int prev=-1;
        sum=0.0;
        int cscan_seq[count[i]];
        find_cscan_sequence(cscan_seq, count[i], rw_head[i], request[i]);
        printf("\nRW HEAD: %d PLATTER %d: ", rw_head[i], i);
        for(int j=0; j<count[i]; j++)
        {
            int idx=cscan_seq[j];
            int id=request[i][idx].id;
            printf("%d ", id);
            cscan->response_time[id]=find_response_time(r, Ts, rw_head[i], request[i][idx].cylinder, request[i][idx].number);
            prev=id;
            rw_head[i]=request[i][idx].cylinder; // Update the RW head
            sum+=cscan->response_time[id];
        }
        if(sum>max_response_time)
        max_response_time=sum;
    }
    printf("\n");
    max_response_time/=1000; // ms to seconds
    return (double)MAX_REQUEST/max_response_time;
}

int main(int argc, char*argv[])
{
    srand(time(0));
    if(argc < 4)
    {
        printf("Info: CLI Arguments should be of the type ./[program_name] [revolutions/minute(r)] [sector size(n)] [average seek time(Ts)]\n");
        exit(0);
    }

    // Extract arguments
    int r=atoi(argv[1]); // revolutions/minute
    int n=atoi(argv[2]); // sector size (in bytes)
    int Ts=atoi(argv[3]); // average seek time (in ms) 

    // Initialize Read/Write head position for each platter: The RW head will have the cylinder number 
    int rw_head[MAX_PLATTER];
    initialize_readwrite_head(rw_head);

    // Initialize request
    struct request_info request[MAX_PLATTER][MAX_REQUEST]; // One for each platter
    int count[MAX_PLATTER]; // Count of request for each platter
    for(int i=0; i<MAX_PLATTER; i++)
    count[i]=0; 
    initialize_request(request);

    // Generate random set of requests
    generate_random_request(request, count);
    print_request(request);

    // Initilaize Scheduling Policy info
    struct scheduling_policy random, fifo, sstf, scan, cscan;
    initialize_scheduling_policy(&random, &fifo, &sstf, &scan, &cscan);

    // Random
    initialize_readwrite_head(rw_head);
    random.throughput=random_scheduler(request, count, &random, rw_head, r, n, Ts);
    // FIFO
    initialize_readwrite_head(rw_head);
    fifo.throughput=fifo_scheduler(request, count, &fifo, rw_head, r, n, Ts);
    // SSTF
    initialize_readwrite_head(rw_head);
    sstf.throughput=sstf_scheduler(request, count, &sstf, rw_head, r, n, Ts);
    // SCAN
    initialize_readwrite_head(rw_head);
    scan.throughput=scan_scheduler(request, count, &scan, rw_head, r, n, Ts);
    // CSCAN
    initialize_readwrite_head(rw_head);
    cscan.throughput=cscan_scheduler(request, count, &cscan, rw_head, r, n, Ts);

    // RESULTS
    printf("\nRANDOM RESPONSE TIME: \n");
    print_result(&random);
    printf("\nFIFO RESPONSE TIME: \n");
    print_result(&fifo);
    printf("\nSSTF RESPONSE TIME: \n");
    print_result(&sstf); 
    printf("\nSCAN RESPONSE TIME: \n");
    print_result(&scan); 
    printf("\nCSCAN RESPONSE TIME: \n");
    print_result(&cscan);
    return 0;
}
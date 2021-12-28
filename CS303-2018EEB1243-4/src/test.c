#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<time.h>
#include<limits.h>
#include<math.h>
#include"function.h"

#define MAX_REQUEST 10 // Can be changed as per need. Make sure to change it in main.c and function.c as well. Same goes for MAX_PLATTER MAX_CYLINDER & MAX_SECTOR
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

int find_sst_request(int platter, struct request_info request[MAX_REQUEST], struct scheduling_policy *sstf, int count, int rw_head);
void initialize_request(struct request_info request[MAX_PLATTER][MAX_REQUEST]);
void initialize_scheduling_policy(struct scheduling_policy *random, struct scheduling_policy *fifo, struct scheduling_policy *sstf, struct scheduling_policy *scan, struct scheduling_policy *cscan);
void generate_random_request(struct request_info request[MAX_PLATTER][MAX_REQUEST], int count[MAX_PLATTER]);
void find_scan_sequence(int scan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST]);
void find_cscan_sequence(int cscan_seq[], int count, int rw_head, struct request_info request[MAX_REQUEST]);

void test_data_transfer_time()
{
    printf("*********************Testing data_transfer_time()*********************\n\n");
    double time = data_transfer_time(2, 7500);
    if(time==0.8)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_response_time()
{
    printf("*********************Testing find_response_time()*********************\n\n");
    double time = find_response_time(7500, 4, 10, 4, 4);
    if(time==24.0+4.0+1.6)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_sst_request()
{
    printf("*********************Testing find_sst_request()*********************\n\n");
    int platter=0, count=4, rw_head=3;
    struct scheduling_policy algo;
    initialize_scheduling_policy(&algo, &algo, &algo, &algo, &algo);
    struct request_info request[MAX_PLATTER][MAX_REQUEST];
    initialize_request(request);
    request[platter][0].cylinder=10;
    request[platter][1].cylinder=1;
    request[platter][2].cylinder=6;
    request[platter][3].cylinder=7;
    int request_idx = find_sst_request(platter, request[0], &algo, count, rw_head);
    if(request_idx==1)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
    rw_head=1;
    request[platter][1].cylinder=9;
    request_idx = find_sst_request(platter, request[0], &algo, count, rw_head);
    if(request_idx==2)
    {
        printf("[+]Test Case 2: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 2: Failed\n\n");
    }
}

void test_generate_random_request()
{
    printf("*********************Testing generate_random_request()*********************\n\n");
    struct request_info request[MAX_PLATTER][MAX_REQUEST];
    initialize_request(request);
    int count[MAX_PLATTER];
    for(int i=0; i<MAX_PLATTER; i++)
    count[i]=0; 

    bool correct=true;
    for(int i=0; i<MAX_PLATTER; i++)
    {
        for(int j=0; j<MAX_REQUEST; j++)
        {
            if(request[i][j].cylinder>=MAX_CYLINDER || request[i][j].sector>=MAX_SECTOR || request[i][j].number>MAX_SECTOR)
            {
                correct=false;
                break;
            }
        }
    }
    if(correct)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_random_sequence()
{
    printf("*********************Testing find_random_sequence()*********************\n\n");
    int random[5], number[5], count=5;
    find_random_sequence(random, count);
    bool correct=true;

    for(int i=0; i<count; i++)
    {
        number[i]=0;
    }

    for(int i=0; i<count; i++)
    {
        number[random[i]]=1;
    }

    for(int i=0; i<count; i++)
    {
        if(number[i]==0)
        {
            correct=false;
            break;
        }
    }

    if(correct)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_scan_sequence()
{
    printf("*********************Testing find_scan_sequence()*********************\n\n");
    int count=5, scan_seq[count], rw_head=3;
    struct request_info request[MAX_REQUEST];
    request[0].cylinder=2;
    request[1].cylinder=1;
    request[2].cylinder=6;
    request[3].cylinder=12;
    request[4].cylinder=3;
    bool correct=true;
    int order[5]={4, 2, 3, 0, 1};
    find_scan_sequence(scan_seq, count, rw_head, request);

    for(int i=0; i<count; i++)
    {
        if(scan_seq[i]!=order[i])
        {
            correct=false;
            break;
        }
    }
    if(correct)
    {
        printf("[+]Test Case 1: Passed\n\n");
    }
    else
    {
        printf("[-]Test Case 1: Failed\n\n");
    }
}

void test_find_cscan_sequence()
{
    printf("*********************Testing find_cscan_sequence()*********************\n\n");
    int count=5, cscan_seq[count], rw_head=3;
    struct request_info request[MAX_REQUEST];
    request[0].cylinder=2;
    request[1].cylinder=1;
    request[2].cylinder=6;
    request[3].cylinder=12;
    request[4].cylinder=3;
    bool correct=true;
    int order[5]={4, 2, 3, 1, 0};
    find_cscan_sequence(cscan_seq, count, rw_head, request);

    for(int i=0; i<count; i++)
    {
        if(cscan_seq[i]!=order[i])
        {
            correct=false;
            break;
        }
    }
    if(correct)
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
    test_generate_random_request();
    test_data_transfer_time();
    test_find_response_time();
    test_find_random_sequence();
    test_find_sst_request();
    test_find_scan_sequence();
    test_find_cscan_sequence();
}
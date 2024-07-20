#include <bits/stdc++.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <semaphore.h>

// Global Variables are declared here
int kw;   // Number of times a writer enters CS
int kr;   // Number of times a reader enters CS
int nw;   // Number of writers
int nr;   // Number of readers
int ucs;  // Mean of CS time
int urem; // Mean of Remaining section time
int randCSTime = 10;
int randRemTime = 5;
using namespace std;

// Global arrays to store the execution times
vector<double> *readertime;
vector<double> *writertime;
vector<double> *worstcasereader;
vector<double> *worstcasewriter;

FILE *fptr = fopen("FairRW-log.txt", "w+");       // Log file to display order of execution of Readers/writers
FILE *outfile = fopen("Average_Time2.txt", "w+"); // File to print average time and worst case time of each thread

// Function that returns minutes and seconds as string
string getSysTime()
{
    time_t now = time(nullptr);
    struct tm *timeinfo;
    char buffer[9]; // "hh:mm:ss\0"

    timeinfo = localtime(&now);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return string(buffer);
}

// Generate a random number from exponential distribution
double generate_exponential(double lambda)
{
    lambda = 1 / (lambda * 1e-3);
    double u = ((double)rand() / RAND_MAX); // Uniform random variable
    return -log(1 - u) / lambda;
}

// Lock structure containing the required variables and locks
typedef struct
{
    int readers;      // Number of active readers
    sem_t commonlock; // Lock to control read/write permission
    sem_t readlock;   // lock to make 'readers' atomic
    sem_t rwqueue;    // Maintains request to CS in FIFO order
} wrlock;

void initialiselock(wrlock *a)
{
    // Intialize number of active readers to 0
    a->readers = 0;

    // intiaslize all semaphores to 1
    sem_init(&a->commonlock, 0, 1);
    sem_init(&a->readlock, 0, 1);
    sem_init(&a->rwqueue, 0, 1);
}

wrlock a;

void acquire_readlock(wrlock *a)
{
    sem_wait(&a->rwqueue);  // wait in the queue to make request
    sem_wait(&a->readlock); // making 'readers' atomical
    a->readers++;           // Increase number of active readers
    if (a->readers == 1)
    {
        sem_wait(&a->commonlock); // If it's the first reader, lock the write permission
    }
    sem_post(&a->readlock); // Release the lock
    sem_post(&a->rwqueue);  // Let the next one in the queue make request
}

void release_readlock(wrlock *a)
{
    sem_wait(&a->readlock); // making 'readers' atomical
    a->readers--;           // Decrease the number of active readers
    if (a->readers == 0)
    {
        sem_post(&(a->commonlock)); // It it's the last reader, release the write permission
    }
    sem_post(&a->readlock); // Release the lock
}

void acquire_writelock(wrlock *a)
{
    sem_wait(&a->rwqueue);    // wait in the queue to make request
    sem_wait(&a->commonlock); // wait for write permission, after acquiring lock the permission to all other threads
}

void release_writelock(wrlock *a)
{
    sem_post(&a->commonlock); // release permission to other threads
    sem_post(&a->rwqueue);    // Let the next one in the queue make request
}

void *writer(void *args)
{
    int id = *((int *)args);
    for (int i = 0; i < kw; i++)
    {
        auto reqTime = getSysTime();
        auto begin = chrono::high_resolution_clock::now();
        fprintf(fptr, "%dth CS request by Writer Thread %d at %s\n", i, id, reqTime.c_str());

        acquire_writelock(&a); // Acquire write lock

        // Begin : Critical Section
        auto enterTime = getSysTime();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - begin;
        (*writertime)[id] += elapsed.count();
        (*worstcasewriter)[id] = max((*worstcasewriter)[id], elapsed.count());

        fprintf(fptr, "%dth CS Entry by Writer Thread %d at %s\n", i, id, enterTime.c_str());

        randCSTime = generate_exponential(ucs);
        sleep(randCSTime); // Simulate a thread writing in CS

        auto exitTime = getSysTime();
        fprintf(fptr, "%dth CS Exit by Writer Thread %d at %s\n", i, id, exitTime.c_str());
        // End : Critical Section

        release_writelock(&a); // Release write lock

        randRemTime = generate_exponential(urem);
        sleep(randRemTime);
    }
    pthread_exit(0);
}
void *reader(void *args)
{
    // pthread_t id = pthread_self();
    int id = *((int *)args);
    for (int i = 0; i < kr; i++)
    {
        auto reqTime = getSysTime();
        auto begin = std::chrono::high_resolution_clock::now();
        fprintf(fptr, "%dth CS request by Reader Thread %d at %s\n", i, id, reqTime.c_str());

        acquire_readlock(&a); // Acquire read lock

        // Begin : Critical Section
        auto enterTime = getSysTime();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - begin;
        (*readertime)[id] += elapsed.count();
        (*worstcasereader)[id] = max((*worstcasereader)[id], elapsed.count());

        fprintf(fptr, "%dth CS Entry by Reader Thread %d at %s\n", i, id, enterTime.c_str());

        randCSTime = generate_exponential(ucs);
        sleep(randCSTime); // Simulate a thread reading in CS

        auto exitTime = getSysTime();
        fprintf(fptr, "%dth CS Exit by Reader Thread %d at %s\n", i, id, exitTime.c_str());
        // End : Critical section

        release_readlock(&a); // release read lock

        randRemTime = generate_exponential(urem);
        sleep(randRemTime); // simulate a thread executing in remainder section
    }
    pthread_exit(0);
}
int main()
{
    // taking inputs
    FILE *inpfile = fopen("inp-params.txt", "r+");
    fstream timesfile("time2.txt", ios::in | ios::out | ios::app);
    fscanf(inpfile, "%d%d%d%d%d%d", &nw, &nr, &kw, &kr, &ucs, &urem);

    // Intialize all variables
    initialiselock(&a);
    readertime = new vector<double>(nr, 0);
    writertime = new vector<double>(nw, 0);
    worstcasereader = new vector<double>(nr, INT_MIN);
    worstcasewriter = new vector<double>(nw, INT_MIN);

    // Intializing Reader and Writer therads
    pthread_t reader_threads[nr], writer_threads[nw];

    // Creating reader threads
    for (int i = 0; i < nr; i++)
    {
        int *args = new int{i};                                           // passing thread index as argument
        pthread_create(&reader_threads[i], NULL, reader, (void *)(args)); // creates thread
    }

    // Creating writer threads
    for (int i = 0; i < nw; i++)
    {
        int *args = new int{i};                                           // passing thread index as argument
        pthread_create(&writer_threads[i], NULL, writer, (void *)(args)); // creates thread
    }

    // Joining reader threads
    for (int i = 0; i < nr; i++)
    {
        pthread_join(reader_threads[i], NULL);
    }

    // Joining writer threads
    for (int i = 0; i < nw; i++)
    {
        pthread_join(writer_threads[i], NULL);
    }

    // Printing Average time of each Reader
    double sum = 0;
    for (int i = 0; i < nr; i++)
    {
        fprintf(outfile, "Average Time for Reader %d to enter CS(msec): %lf\n", i, ((*readertime)[i] / kr)*1e3);
        sum += (*readertime)[i] / kr;
    }
    sum = sum / nr;

    // Printing Average time of each Writer
    double sum2 = 0;
    for (int i = 0; i < nw; i++)
    {
        fprintf(outfile, "Average Time for Writer %d to enter CS(msec): %lf\n", i, ((*writertime)[i] / kw)*1e3);
        sum2 += (*writertime)[i] / kw;
    }
    sum2 = sum2 / nw;

    // Printing Worst time of each Reader
    double sum3 = 0;
    for (int i = 0; i < nr; i++)
    {
        fprintf(outfile, "Worst Case Time for Reader %d to enter CS(msec): %lf\n", i, ((*worstcasereader)[i])*1e3);
        sum3 += (*worstcasereader)[i];
    }
    sum3 = sum3 / nr;

    // Printing Worst time of each Writer
    double sum4 = 0;
    for (int i = 0; i < nw; i++)
    {
        fprintf(outfile, "Worst Case Time for Writer %d to enter CS(msec): %lf\n", i, ((*worstcasewriter)[i])*1e3);
        sum4 += (*worstcasewriter)[i];
    }
    sum4 = sum4 / nw;

    // Printing overall average of reader and writer
    timesfile << nw << " " << nr << " " << kw << " " << kr << " " << ucs << " " << urem << " R " << sum << " " << sum3 << endl;
    timesfile << nw << " " << nr << " " << kw << " " << kr << " " << ucs << " " << urem << " W " << sum2 << " " << sum4 << endl;

    // Closing files
    timesfile.close();
    fclose(fptr);
    fclose(inpfile);
    return 0;
}
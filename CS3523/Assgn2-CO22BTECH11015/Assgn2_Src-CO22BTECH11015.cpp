/*******************************************************
*             Name: Varun Gopal                        *
*             Roll No: CO22BTECH11015                  *
*             OS 2 Assignment-2                        *
*                                                      *
********************************************************/
#include <bits/stdc++.h>
#include <ctime>
#include <pthread.h>
using namespace std;

// The 'a' variable stores the matrix that needs to be squared
// c1, c2 correspond to the outputs of methods 1,2 respectively
int **a;
int **c1;
int **c2;
int **c3;

// Function which returns the dotproduct of a row and column according to i,j and matrix a
int dot(int i, int j, int **a, int n)
{
    int ans = 0;
    for (int k = 0; k < n; k++)
    {
        ans += a[i][k] * a[k][j];
    }
    return ans;
}

// Structure used to pass the range of rows to be passed onto each thread in method 1
struct node
{
    int s;
    int e;
    int **a;
    int n;
    int core;

    node(int sa, int ea, int **aa, int na)
    {
        s = sa;
        e = ea;
        a = aa;
        n = na;
    }
};
typedef struct node node;

// Structure used to pass the range of rows to be passed onto each thread in method 2
struct node2
{
    vector<int> range;
    int **a;
    int n;
    int core;
};
typedef struct node2 node2;

double tavg1=0;
double tavg2=0;
double tavg3=0;
double tavg4=0;

// The function which computes the values of resultant matrix of rows passed as args in Method 1
void *m1(void *param)
{
    auto t1 = clock();
    node *args = (node *)param;
    int s = args->s;
    int e = args->e;
    int n = args->n;
    int **a = args->a;
    int pinCore=args->core;
// Assigning thread affinity to itself
    pthread_t self = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if(pinCore!=-1)
    {
    CPU_SET(pinCore, &cpuset);
    int rc = pthread_setaffinity_np(self, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        // If the assigning of thread to a CPU fails
        printf("Failed to pin core: %s\n", strerror(errno));
        exit(1);
    }

    printf("#%d Running: CPU %d\n", pinCore, sched_getcpu());
    }

    for (int i = s; i <= e; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c1[i][j] = dot(i, j, a, n);
        }
    }
    auto t2 = clock();
    double duration = static_cast<double>(t2 - t1) / CLOCKS_PER_SEC;
    if(pinCore==-1)
    {
        tavg1+=duration;
    }
    else{
        tavg2+=duration;
    }
    pthread_exit(0);
}

// The function which computes the values of resultant matrix of rows passed as args in Method 2
void *m2(void *param)
{
    auto t1=clock();
    node2 *args = (node2 *)param;
    vector<int> v = args->range;
    int n = args->n;
    int **a = args->a;
    int pinCore=args->core;
    // Assigning thread affinity to itself
    pthread_t self = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if(pinCore!=-1)
    {
    CPU_SET(pinCore, &cpuset);
    int rc = pthread_setaffinity_np(self, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        // If the assigning of thread to a CPU fails
        printf("Failed to pin core: %s\n", strerror(errno));
        exit(1);
    }

    printf("#%d Running: CPU %d\n", pinCore, sched_getcpu());
    }
    for (int i : v)
    {
        for (int j = 0; j < n; j++)
        {
            c2[i][j] = dot(i, j, a, n);
        }
    }
    auto t2 = clock();
    double duration = static_cast<double>(t2 - t1) / CLOCKS_PER_SEC;
    if(pinCore==-1)
    {
        tavg3+=duration;
    }
    else{
        tavg4+=duration;
    }
    pthread_exit(0);
}


int main()
{
    clock_t start, stop;
    double duration;
    // N, K, Array A are taken as input from the file input.txt
    FILE *fptr = fopen("inp.txt", "r+");
    int n;
    int k;
    int C;
    int BT;

    fscanf(fptr, "%d %d %d %d", &n, &k,&C,&BT);
    // Creating an array to store thread ids of all threads created and joined for method 1
    pthread_t thrarr[k];
    // Dynamic memory Allocation of suitable amount of memory (n*n) to 'a'
    a = new int *[n];
    for (int i = 0; i < n; i++)
    {
        a[i] = new int[n];
    }
    // Dynamic memory Allocation of suitable amount of memory (n*n) to 'c1'
    c1 = new int *[n];
    for (int i = 0; i < n; i++)
    {
        c1[i] = new int[n];
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fscanf(fptr, "%d", &a[i][j]);
        }
    }
    int s;
    int e;
    FILE *fptr1 = fopen("outFile1.txt", "w+");
    /*
        Method 1
    */
    // Creating K threads for Method 1 and assigning suitable rows(1 to N/K,N/K+1 to 2N/K,...) to each thread
    start = clock();
    for (int y = 0; y < k; y++)
    {
        node *args = new node(y * n / k, y * n / k + n / k - 1, a, n);
        if (y == k - 1)
        {
            args->e += n % k;
        }
        args->core=-1;
        int u=k/C;
        if(y<BT)
        {
        args->core=y%C;
        }
        
        // Passing the function to be executed in thread and the range of rows as arguments for thread creation
        pthread_create(&thrarr[y], NULL, m1, (void *)args);
    }
    // waiting for all threads to finish their execution

    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr[i], NULL);
    }

    stop = clock();
    duration = static_cast<double>(stop - start) / CLOCKS_PER_SEC;

    // Output the value of the time of execution for method 1 to complete matrix multiplication and the result
    fprintf(fptr1, "Time Elapsed for Method 1: %lf \n", duration);


    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fprintf(fptr1, "%d ", c1[i][j]);
        }
        fprintf(fptr1, "\n");
    }

    fclose(fptr1);

    /*
    Method 2
    */
    // Allocation of memory to the c2 matrix to store result of method 2
    c2 = new int *[n];
    for (int i = 0; i < n; i++)
    {
        c2[i] = new int[n];
    }

    // Creating an array to store thread ids of all threads created and joined for method 2
    pthread_t thrarr2[k];

    FILE *fptr2 = fopen("outFile2.txt", "w+");

    start = clock();
    // Grouping suitable range of rows and preparing them to be passed to the threads
    for (int i = 0; i < k; i++)
    {
        node2 *args = new node2();
        args->a = a;
        args->n = n;
        vector<int> &v = args->range;
        for (int y = 0; y < n / k; y++)
        {
            v.push_back(y * k + i);
        }
        if (i == k - 1)
        {
            for (int i = 0; i < n % k; i++)
            {
                v.push_back(n - i - 1);
            }
        }
        // Passing the Cpu core for each thread to be assigned. -1 for default OS scheduling
        args->core=-1;
        int u=k/C;
        if(i<BT)
        {
        args->core=i%C;
        }
        // Create threads and pass the range of rows to be evaluated
        pthread_create(&thrarr2[i], NULL, m2, (void *)args);
    }
    // Waiting for threads to complete the execution
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr2[i], NULL);
    }

    stop = clock();
    duration = static_cast<double>(stop - start) / CLOCKS_PER_SEC;

    // Finding Time of execution and printing resultant matrix into a file
    fprintf(fptr2, "Time Elapsed for Method 2: %lf \n", duration);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fprintf(fptr2, "%d ", c2[i][j]);
        }
        fprintf(fptr2, "\n");
    }
    fclose(fptr2);



    cout << n << " " << k << endl;
}
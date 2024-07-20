/*******************************************************
*             Name: Varun Gopal                        *
*             Roll No: CO22BTECH11015                  *
*             OS 2 Assignment-1                        *
*                                                      *
********************************************************/
#include <bits/stdc++.h>
#include <ctime>
#include <pthread.h>
using namespace std;

// The 'a' variable stores the matrix that needs to be squared
// c1, c2, c3 correspond to the outputs of methods 1,2 and 3 respectively
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

    node(int sa, int ea, int **aa, int na)
    {
        s = sa;
        e = ea;
        a = aa;
        n = na;
    }
};
typedef struct node node;

// Structure used to pass the range of rows to be passed onto each thread in method 2 and 3
struct node2
{
    vector<int> range;
    int **a;
    int n;
};
typedef struct node2 node2;

// The function which computes the values of resultant matrix of rows passed as args in Method 1
void *m1(void *param)
{
    node *args = (node *)param;
    int s = args->s;
    int e = args->e;
    int n = args->n;
    int **a = args->a;

    for (int i = s; i <= e; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c1[i][j] = dot(i, j, a, n);
        }
    }
    pthread_exit(0);
}

// The function which computes the values of resultant matrix of rows passed as args in Method 2
void *m2(void *param)
{
    node2 *args = (node2 *)param;
    vector<int> v = args->range;
    int n = args->n;
    int **a = args->a;

    for (int i : v)
    {
        for (int j = 0; j < n; j++)
        {
            c2[i][j] = dot(i, j, a, n);
        }
    }
    pthread_exit(0);
}

// The function which computes the values of resultant matrix of rows passed as args in Method 3
void *m3(void *param)
{
    node2 *args = (node2 *)param;
    vector<int> v = args->range;
    int n = args->n;
    int **a = args->a;

    for (int i : v)
    {
        for (int j = 0; j < n; j++)
        {
            c3[i][j] = dot(i, j, a, n);
        }
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
    fscanf(fptr, "%d %d", &n, &k);
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
    // Pre Computations of norms of each row so that they can be sorted and rows can be equally distributed
    // to each thread.
    vector<pair<int, int>> norms(n);
    for (int i = 0; i < n; i++)
    {
        norms[i].first = 0;
        norms[i].second = i;
        for (int j = 0; j < n; j++)
        {
            fscanf(fptr, "%d", &a[i][j]);
            norms[i].first += a[i][j] * a[i][j];
        }
    }
    sort(norms.begin(), norms.end());
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

    /*
        Method 3
    */
    // Allocating memory to the c3 matrix to store output of method 3
    c3 = new int *[n];
    for (int i = 0; i < n; i++)
    {
        c3[i] = new int[n];
    }
    // Initialising the matrix c3
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            c3[i][j] = 0;
        }
    }

    // Creating a multidimensional vector to store the rows that are to be assigned to each thread
    vector<vector<int>> m3arr(k);
    deque<int> dq;

    // Using a deque to uniformly distribute the vectors based on their norms
    for (int i = 0; i < n; i++)
    {
        dq.push_back(norms[i].second);
    }
    int i = 0;
    while (!dq.empty())
    {
        for (int l = 0; l < k; l++)
        {
            if (dq.empty())
            {
                break;
            }
            int y = dq.front();
            dq.pop_front();
            m3arr[l].push_back(y);
        }

        for (int l = 0; l < k; l++)
        {
            if (dq.empty())
            {
                break;
            }
            int j = dq.back();
            dq.pop_back();
            m3arr[l].push_back(j);
        }
    }

    //Initialising threads to store the thread id's
    pthread_t thrarr3[k];
    start=clock();
    for (int i = 0; i < k; i++)
    {
        node2 *ne = new node2;
        ne->a = a;
        ne->n = n;
        ne->range = m3arr[i];
        // Creating threads and assigning them their respective ranges
        pthread_create(&thrarr3[i], NULL, m3, (void *)ne);
    }

    //Waiting for all the threads to finish their execution
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr3[i], NULL);
    }

    stop = clock();
    duration = static_cast<double>(stop - start) / CLOCKS_PER_SEC;

    // Finding Time of execution and printing resultant matrix into a file


    FILE *fptr3 = fopen("outFile3.txt", "w+");
    fprintf(fptr3, "Time Elapsed for Method 3: %lf \n", duration);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fprintf(fptr3, "%d ", c3[i][j]);
        }
        fprintf(fptr3, "\n");
    }

    fclose(fptr3);
    cout << n << " " << k << endl;
}
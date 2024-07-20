#include<bits/stdc++.h>
#include<fstream>
#include <atomic>
#include <ctime>
#include <pthread.h>
#include <sys/time.h>
using namespace std;

int lk=0;
int lk2=0;
bool* waiting;
int lk3(0);
std::atomic<int> lk4(0);

int **a;
int **c;
int n,k,rowinc;
int counter=-1;
int counter2=-1;
int counter3=-1;

struct node{
    int i;
};

typedef struct node node;
int dot(int l,int r)
{
    int sum=0;
    for(int i=0;i<n;i++)
    {
        sum+=a[l][i]*a[i][r];
    }
    return sum;
}

int test_set(int *lck)
{
    return __sync_lock_test_and_set(lck, 1);
}


void* runner(void* args)
{
    int l=0;
    int r=n-1;
while(1)
{ 
    while (test_set(&lk)) {};
    l=counter+1;
    counter=min(n-1,counter+rowinc);
    lk=0;
    r=min(l+rowinc,n-1);
    if(l>=n)
    {
        pthread_exit(0);
    }
    for(int i=l;i<=r;i++)
    {
        for(int j=0;j<n;j++)
        {
            c[i][j]=dot(i,j);
        }
    }
}
    pthread_exit(0);
}

void* runner2(void* args)
{
    int l=0;
    int r=n-1;
while(1)
{ 
    while (__sync_val_compare_and_swap(&lk2, 0, 1) != 0) {}
    l=counter2+1;
    counter2=min(n-1,counter2+rowinc);
    lk2=0;

    r=min(l+rowinc,n-1);
    if(l>=n)
    {
        pthread_exit(0);
    }
    for(int i=l;i<=r;i++)
    {
        for(int j=0;j<n;j++)
        {
            c[i][j]=dot(i,j);
        }
    }
}
    pthread_exit(0);
}

void* runner3(void* args)
{
    node * avs=(node*)args;
    int i=avs->i;
    int l=0;
    int r=n-1;
while(1)
{ 
    waiting[i]=true;
    int key=1;
    while(waiting[i]&&key==1)
    {
        key=__sync_val_compare_and_swap(&lk3, 0, 1);
    }
    waiting[i]=false;
// Critical section starts
    l=counter3+1;
    counter3=min(n-1,counter3+rowinc);
// Critical section ends
    int j=(i+1)%k;
    while(j!=i&&(!waiting[j]))
    {
        j=(j+1)%k;
    }
    if(j==i)
    {
        lk3=0;
    }
    else{
        waiting[i]=false;
    }

    r=min(l+rowinc,n-1);
    if(l>=n)
    {
        pthread_exit(0);
    }
    for(int i=l;i<=r;i++)
    {
        for(int j=0;j<n;j++)
        {
            c[i][j]=dot(i,j);
        }
    }
}
    pthread_exit(0);
}


void* runner4(void* args)
{
    int l=0;
    int r=n-1;
while(1)
{ 
    l=lk4.fetch_add(rowinc);
    cout<<l;
    if(l>=n)
    {
        pthread_exit(0);
    }
    r=min(l+rowinc,n-1);
    

    for(int i=l;i<=r;i++)
    {
        for(int j=0;j<n;j++)
        {
            c[i][j]=dot(i,j);
        }
    }
}
    pthread_exit(0);
}


int main()
{
    struct timeval start,stop;
    fstream fptr,fptr2;
    fptr.open("input.txt",ios::in|ios::out|ios::app);
    double times[4];
    fptr>>n>>k>>rowinc;
    cin>>n>>k>>rowinc;
    a=new int*[n];
    c=new int*[n];
    waiting=new bool[k];
    for(int i=0;i<k;i++)
    {
        waiting[i]=false;
    }
    for(int i=0;i<n;i++)
    {
        a[i]=new int[n];
        c[i]=new int[n];
    }
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fptr>>a[i][j];
        }
    }
    
    fptr.close();
    pthread_t thrarr[k];
    void * dummy;
    
    gettimeofday(&start, NULL);


    for(int i=0;i<k;i++)
    {
        pthread_create(&thrarr[i], NULL, runner,dummy);
    }
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr[i], NULL);
    }

    gettimeofday(&stop, NULL);

    long seconds = stop.tv_sec - start.tv_sec;
    long microseconds = stop.tv_usec - start.tv_usec;
    double duration = seconds + microseconds / 1000000.0;

    fptr2.open("outFile1.txt",ios::trunc|ios::in|ios::out);
    times[0]=duration;
    fptr2<<"Time Elapsed for the Computations for TAS: "<<times[0]<<endl;
for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fptr2<<c[i][j]<<((j==n-1)? "\n" : " ");
            c[i][j]=-8;
        }
    }
    fptr2.close();


    pthread_t thrarr2[k];
    gettimeofday(&start, NULL);
    for(int i=0;i<k;i++)
    {
        pthread_create(&thrarr2[i], NULL, runner2,dummy);
    }
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr2[i], NULL);
    }

    gettimeofday(&stop, NULL);

    seconds = stop.tv_sec - start.tv_sec;
    microseconds = stop.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;

    fptr2.open("outFile2.txt",ios::trunc|ios::in|ios::out);
    times[1]=duration;
    fptr2<<"Time Elapsed for the Computations for CAS: "<<times[1]<<endl;
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fptr2<<c[i][j]<<((j==n-1)? "\n" : " ");
            c[i][j]=-8;
        }
    }
        fptr2.close();
       


    pthread_t thrarr3[k];
    gettimeofday(&start, NULL);
    struct node* h[k];
    for(int i=0;i<k;i++)
    {       
        h[i]=new node;
        h[i]->i=i;
        pthread_create(&thrarr3[i], NULL, runner3,(void* )(h[i]));
    }
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr3[i], NULL);
    }
    gettimeofday(&stop, NULL);

    seconds = stop.tv_sec - start.tv_sec;
    microseconds = stop.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    fptr2.open("outFile3.txt",ios::trunc|ios::in|ios::out);
    times[2]=duration;
    fptr2<<"Time Elapsed for the Computations for Bounded CAS: "<<times[2]<<endl;
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fptr2<<c[i][j]<<((j==n-1)? "\n" : " ");
            c[i][j]=-8;
        }
    }
    fptr2.close();
    pthread_t thrarr4[k];
        gettimeofday(&start, NULL);
    for(int i=0;i<k;i++)
    {
        pthread_create(&thrarr4[i], NULL, runner4,dummy);
    }
    for (int i = 0; i < k; i++)
    {
        pthread_join(thrarr4[i], NULL);
    }
        gettimeofday(&stop, NULL);
    seconds = stop.tv_sec - start.tv_sec;
    microseconds = stop.tv_usec - start.tv_usec;
    duration = seconds + microseconds / 1000000.0;
    fptr2.open("outFile4.txt",ios::trunc|ios::in|ios::out);
    times[3]=duration;
    fptr2<<"Time Elapsed for the Computations for Atomic increment: "<<times[3]<<endl;
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<n;j++)
        {
            fptr2<<c[i][j]<<((j==n-1)? "\n" : " ");
        }
    }

    fptr2.close();
    fstream fptr3;
    fptr3.open("times.txt",ios::app|ios::in|ios::out);
    fptr3<<n<<" "<<k<<" "<<rowinc<<" "<<times[0]<<" "<<times[1]<<" "<<times[2]<<" "<<times[3];
    fptr3<<endl;
    fptr3.close();


    
    return 0;
}
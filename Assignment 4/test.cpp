// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// using namespace std;
// typedef struct
// {
//         int *a;
//         int length;
//         int sum;
// } MyData;

// #define N 5
// #define L 20

// MyData mData;
// pthread_t myThread[N];
// pthread_mutex_t mutex;

// void *threadWork(void *arg)
// {
//         /* Define and use local variables for convenience */
//         long offset = (long)arg;
//         int sum = 0;
//         int start = offset * mData.length;
//         int end = start + mData.length;

//         /* each thread calculates its sum */
//         for (int i = start; i < end ; i++)  sum += mData.a[i];

//         /* mutex lock/unlock */
//         pthread_mutex_lock(&mutex);
//         mData.sum += sum;
//         pthread_mutex_unlock(&mutex);

//         pthread_exit((void*) 0);
// }

// int main ()
// {
//         void *status;

//         /* fill the structure */
//         int *a = (int*) malloc (N*L*sizeof(int));
//         for (int i = 0; i < N*L; i++) a[i] = i + 1;
//         mData.length = L;
//         mData.a = a;
//         mData.sum = 0;

//         pthread_mutex_init(&mutex, NULL);

//         /* Each thread has its own  set of data to work on. */
//         for(long i=0; i < N; i++)
//                 pthread_create(&myThread[i], NULL, threadWork, (void *)i);

//         /* Wait on child threads */
//         for(int i=0; i < N; i++) pthread_join(myThread[i], &status);

//         /* Results and cleanup */
//         printf ("Sum = %d \n", mData.sum);
//         free (a);
//         pthread_mutex_destroy(&mutex); pthread_exit(NULL);
// }
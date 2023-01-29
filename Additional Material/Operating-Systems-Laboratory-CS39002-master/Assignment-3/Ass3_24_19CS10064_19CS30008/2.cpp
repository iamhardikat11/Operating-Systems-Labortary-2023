#include <pthread.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <random>

using namespace std;
using namespace std::chrono;

const int MAX_SLEEP = 3001;
const int QUEUE_SIZE = 9;
const int N = 1000;
int MAX_JOBS;

struct Job {
    int prod_num;         // Producer number that created this job
    int status;           // stores the status of matrix multiplication block-wise
    long long mat[N][N];  // Matrix
    int mat_id;           // Matrix ID, is a random integer

    Job(int num) {
        prod_num = num;
        status = 0;
        mat_id = rand() % 100000 + 1;
    }

    /* 
        Returns a pair of integers, first is a block from the first matrix,
        second is a block from the second matrix that is to be multiplied
    */
    pair<int, int> get_blocks() {
        for (int i = 0; i < 8; i++) {
            if (!(status & (1 << i))) {
                status |= (1 << i);
                return {i / 2, i % 4};
            }
        }
        return {-1, -1};
    }
};

struct SharedMem {
    Job job_queue[QUEUE_SIZE];  // Job queue
    int in;                     // Index of the next job to be added
    int out;                    // Index of the next job to be removed
    int count;                  // Number of jobs in the queue
    int job_created;            // Number of jobs created by the producers
    int write_ind;              // Index of the next job to be written to the shared memory
    pthread_mutex_t mutex;      // Mutex to protect the shared memory

    /*
        Initialize the job queue
    */
    void init() {
        in = 0;
        out = 0;
        count = 0;
        job_created = 0;
        write_ind = 0;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mutex, &attr);
    }
};

// Shared memory
SharedMem *SHM;

/*
    Function to print a matrix
*/
void print_matrix(Job &job) {
    cout << "Matrix: " << endl;
    cout << "[";
    for (int i = 0; i < N; i++) {
        cout << "[";
        for (int j = 0; j < N; j++) {
            cout << job.mat[i][j];
            if (j != N - 1) {
                cout << ", ";
            }
        }
        cout << (i != N - 1 ? "],\n" : "]");
    }
    cout << "]" << endl;
}

/*
    Prints the newly created job details to the console
*/
void producer_out(Job &job) {
    cout << endl;
    cout << "Producer number: " << job.prod_num << endl;
    cout << "pid: " << getpid() << endl;
    cout << "job_created: " << SHM->job_created << endl;
    cout << "Matrix ID: " << job.mat_id << endl;
    // print_matrix(job); // Uncomment to print the matrix
}

/*
    Prints the worker process details to the console
*/
void consumer_out(int num, int a_ind, int b_ind, pair<int, int> blocks) {
    cout << endl;
    cout << "Worker number: " << num << endl;
    cout << "Producer numbers: " << SHM->job_queue[a_ind].prod_num << ", " << SHM->job_queue[b_ind].prod_num << endl;
    cout << "Matrix IDs: " << SHM->job_queue[a_ind].mat_id << " " << SHM->job_queue[b_ind].mat_id << endl;
    cout << "Blocks: " << bitset<2>(blocks.first) << " " << bitset<2>(blocks.second) << endl;
}

/*
    This function is called by the producer to create a new job,
    wait for a random interval of time between 0 and 3 seconds,
    and add it to the job queue
*/
void producer(int num) {
    while (1) {
        pthread_mutex_lock(&SHM->mutex);
        if (SHM->job_created == MAX_JOBS) {
            // If the maximum number of jobs has been created, then exit
            pthread_mutex_unlock(&SHM->mutex);
            break;
        }
        pthread_mutex_unlock(&SHM->mutex);

        // Create a new job to be added to the job queue
        Job job(num);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                job.mat[i][j] = rand() % 19 - 9;
            }
        }

        // Sleep for a random interval of time between 0 and 3 seconds
        int ms = (rand() % MAX_SLEEP) * 1000;
        usleep(ms);

        pthread_mutex_lock(&SHM->mutex);
        // Spin until the job queue is full
        while (SHM->count >= QUEUE_SIZE - 1) {
            pthread_mutex_unlock(&SHM->mutex);
            usleep(1);
            pthread_mutex_lock(&SHM->mutex);
        }
        // Add the new job to the job queue
        if (SHM->job_created < MAX_JOBS) {
            SHM->job_queue[SHM->in] = job;
            SHM->in = (SHM->in + 1) % QUEUE_SIZE;
            SHM->count++;
            SHM->job_created++;
            producer_out(job);
        }
        pthread_mutex_unlock(&SHM->mutex);
    }
}

/*
    This function is called by the workers to process a job,
    wait for a random interval of time between 0 and 3 seconds,
    retrieves two blocks of the first two matrices in the queue, and
    updates the status of the job
*/
void worker(int num) {
    while (1) {
        pthread_mutex_lock(&SHM->mutex);
        // If maximum number of jobs has been created by producers and count of jobs in the SHM queue is 1, then exit
        if (SHM->count == 1 && SHM->job_created == MAX_JOBS) {
            pthread_mutex_unlock(&SHM->mutex);
            break;
        }
        pthread_mutex_unlock(&SHM->mutex);

        // Sleep for a random interval of time between 0 and 3 seconds
        int ms = (rand() % MAX_SLEEP) * 1000;
        usleep(ms);

        pthread_mutex_lock(&SHM->mutex);
        // Spin until the size of job queue < 2
        while (SHM->count < 2 && !(SHM->job_created == MAX_JOBS)) {
            pthread_mutex_unlock(&SHM->mutex);
            usleep(1);
            pthread_mutex_lock(&SHM->mutex);
        }
        if (SHM->count == 1 && SHM->job_created == MAX_JOBS) {
            pthread_mutex_unlock(&SHM->mutex);
            break;
        }

        // Get the two unprocessed blocks of the first two matrices in the queue
        pair<int, int> blocks = SHM->job_queue[SHM->out].get_blocks();

        // If there are blocks to be processed, then process them
        if (blocks != make_pair(-1, -1)) {
            // If matrices A and B are accessed for the first time, then
            // create space for the product matrix in the queue and increment in
            if (blocks == make_pair(0, 0)) {
                SHM->write_ind = SHM->in;
                SHM->in = (SHM->in + 1) % QUEUE_SIZE;
                SHM->count++;
                SHM->job_queue[SHM->write_ind].prod_num = num;
                SHM->job_queue[SHM->write_ind].mat_id = rand() % 100000 + 1;
                SHM->job_queue[SHM->write_ind].status = 0;
            }

            int a_ind = SHM->out;                  // Index of matrix A
            int b_ind = (a_ind + 1) % QUEUE_SIZE;  // Index of matrix B

            // p is the block of matrix C which is to be updated
            int p = 2 * ((blocks.first & 2) + (blocks.second & 1));

            consumer_out(num, a_ind, b_ind, blocks);
            cout << "Reading" << endl;
            pthread_mutex_unlock(&SHM->mutex);

            // Compute the product of the two blocks and store it in pmat
            long long pmat[N / 2][N / 2];
            for (int i = 0; i < N / 2; i++) {
                for (int j = 0; j < N / 2; j++) {
                    pmat[i][j] = 0;
                    for (int k = 0; k < N / 2; k++) {
                        int acol = k + N / 2 * (blocks.first & 1);
                        int arow = i + N / 2 * (blocks.first / 2);
                        int bcol = j + N / 2 * (blocks.second & 1);
                        int brow = k + N / 2 * (blocks.second / 2);
                        pmat[i][j] += SHM->job_queue[a_ind].mat[arow][acol] * SHM->job_queue[b_ind].mat[brow][bcol];
                    }
                }
            }

            pthread_mutex_lock(&SHM->mutex);

            consumer_out(num, a_ind, b_ind, blocks);

            // Copy the contents of pmat into block p of the product matrix in the queue
            // if block p is being accessed for the first time
            if (!(SHM->job_queue[SHM->write_ind].status & (1 << p)) && !(SHM->job_queue[SHM->write_ind].status & (1 << (p + 1)))) {
                cout << "Copying" << endl;
                for (int i = 0; i < N / 2; i++) {
                    for (int j = 0; j < N / 2; j++) {
                        int crow = i + (N / 2) * (blocks.first / 2);
                        int ccol = j + (N / 2) * (blocks.second & 1);
                        SHM->job_queue[SHM->write_ind].mat[crow][ccol] = pmat[i][j];
                    }
                }
                SHM->job_queue[SHM->write_ind].status |= (1 << p);

            }
            // Add the contents of pmat to block p of the product matrix in the queue
            // if block p had already being accessed before
            else if (!(SHM->job_queue[SHM->write_ind].status & (1 << (p + 1)))) {
                cout << "Adding" << endl;
                for (int i = 0; i < N / 2; i++) {
                    for (int j = 0; j < N / 2; j++) {
                        int crow = i + N / 2 * (blocks.first / 2);
                        int ccol = j + N / 2 * (blocks.second & 1);
                        SHM->job_queue[SHM->write_ind].mat[crow][ccol] += pmat[i][j];
                    }
                }
                SHM->job_queue[SHM->write_ind].status |= (1 << (p + 1));
            }

            // If all blocks of the product matrix in the queue are processed, then
            // decrement the count of jobs in the SHM queue and increment out
            if (SHM->job_queue[SHM->write_ind].status == (1 << 8) - 1) {
                SHM->job_queue[SHM->write_ind].status = 0;
                SHM->out = (SHM->out + 2) % QUEUE_SIZE;
                SHM->count -= 2;
                cout << endl
                     << "One step done" << endl;
            }
        }
        pthread_mutex_unlock(&SHM->mutex);
    }
}

int main() {
    srand(time(NULL));

    int NP, NW;
    cout << "Enter number of producers (NP): ";
    cin >> NP;
    cout << "Enter number of workers (NW): ";
    cin >> NW;
    cout << "Enter number of marices to multiply: ";
    cin >> MAX_JOBS;

    auto start = high_resolution_clock::now();

    // Initialize shared memory
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedMem), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    // Attach shared memory
    SHM = (SharedMem *)shmat(shmid, NULL, 0);
    if (SHM == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize the shared memory queue
    SHM->init();

    vector<pid_t> producers, workers;

    // Create producers
    for (int i = 1; i <= NP; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            srand(time(NULL) * getpid());
            producer(i);
            exit(0);
        } else {
            producers.push_back(pid);
        }
    }

    // Create workers
    for (int i = 1; i <= NW; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            srand(time(NULL) * getpid());
            worker(-i);
            exit(0);
        } else {
            workers.push_back(pid);
        }
    }

    // Wait for the termination condition to be reached
    while (1) {
        pthread_mutex_lock(&SHM->mutex);
        if (SHM->count == 1 && SHM->job_created == MAX_JOBS) {
            // Calculate the trace of the product matrix finally remaining in the queue
            long long trace = 0;
            for (int i = 0; i < N; i++) {
                trace += SHM->job_queue[SHM->out].mat[i][i];
            }
            cout << endl
                 << "Trace: " << trace << endl;
            auto end = high_resolution_clock::now();
            // Calculate the time taken by the program
            auto duration = duration_cast<milliseconds>(end - start);
            // Print the time taken by the program
            cout << "Time taken: " << duration.count() << " ms" << endl;
            pthread_mutex_unlock(&SHM->mutex);
            pthread_mutex_destroy(&SHM->mutex);
            for (pid_t pid : producers) {
                kill(pid, SIGKILL);
            }
            for (pid_t pid : workers) {
                kill(pid, SIGKILL);
            }
            break;
        }
        pthread_mutex_unlock(&SHM->mutex);
    }

    // Detach shared memory
    shmdt(SHM);

    // Remove shared memory
    shmctl(shmid, IPC_RMID, NULL);
    exit(0);
}
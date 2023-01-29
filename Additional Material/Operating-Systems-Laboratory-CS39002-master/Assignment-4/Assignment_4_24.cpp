#include <errno.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"

const int MIN_INIT_NODES = 300;
const int MAX_INIT_NODES = 500;
const int MAX_COMP_TIME = 250;
const int MAX_CHILD_JOBS = 20;
const int MAX_ID = (int)1e8;
const int MIN_PROD_TIME = 10;
const int MAX_PROD_TIME = 20;
const int MIN_PROD_SLEEP = 200;
const int MAX_PROD_SLEEP = 500;

int MAX_NODES;

enum JobStatus {
    WAITING,
    ONGOING,
    DONE
};

// Returns a random number between low and high
int rand(int low, int high) {
    return low + rand() % (high - low + 1);
}

// A wrapper around pthread_mutex_lock for error detection
void LOCK(pthread_mutex_t *mutex) {
    int status = pthread_mutex_lock(mutex);
    if (status != 0) {
        printf(COLOR_RED "pthread_mutex_lock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

// A wrapper around pthread_mutex_unlock for error detection
void UNLOCK(pthread_mutex_t *mutex) {
    int status = pthread_mutex_unlock(mutex);
    if (status != 0) {
        printf(COLOR_RED "pthread_mutex_unlock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

struct Node {
    int job_id;
    int comp_time;  // in milliseconds
    int child_jobs[MAX_CHILD_JOBS];
    int child_count;
    int parent;
    JobStatus status;
    pthread_mutex_t mutex;

    Node() {
        job_id = rand(1, MAX_ID);
        comp_time = rand(0, MAX_COMP_TIME);
        for (int i = 0; i < MAX_CHILD_JOBS; i++) {
            child_jobs[i] = -1;
        }
        child_count = 0;
        parent = -1;
        status = WAITING;
    }

    // Implemeting the assignment operator to ensure the mutex doesn't get copied
    Node &operator=(const Node &node) {
        job_id = node.job_id;
        comp_time = node.comp_time;
        for (int i = 0; i < MAX_CHILD_JOBS; i++) {
            child_jobs[i] = node.child_jobs[i];
        }
        child_count = node.child_count;
        parent = node.parent;
        status = node.status;
        return *this;
    }

    // Adds a child job to the node
    int addChild(int child_id) {
        if (child_count >= MAX_CHILD_JOBS) {
            return -1;
        }
        for (int i = 0; i < MAX_CHILD_JOBS; i++) {
            if (child_jobs[i] == -1) {
                child_jobs[i] = child_id;
                child_count++;
                return i;
            }
        }
        return -1;
    }

    // Sets the parent link of a node
    void setParent(int parent_id) {
        parent = parent_id;
    }

    // Deletes a child job from the node
    int deleteChild(int child_id) {
        for (int i = 0; i < MAX_CHILD_JOBS; i++) {
            if (child_jobs[i] == child_id) {
                child_jobs[i] = -1;
                child_count--;
                return i;
            }
        }
        return -1;
    }
};

struct SharedMem {
    Node *tree;      // The tree stored as an array of nodes
    int node_count;  // Current number of nodes in the tree
    int root;        // Root node of the tree
    pthread_mutex_t mutex;

    void init() {
        node_count = 0;
        root = -1;
        for (int i = 0; i < MAX_NODES; i++) {
            tree[i].status = DONE;
            for (int j = 0; j < MAX_CHILD_JOBS; j++) {
                tree[i].child_jobs[j] = -1;
            }
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&tree[i].mutex, &attr);
        }
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mutex, &attr);
    }

    // Adds a new node to the tree
    int addNode(Node &node) {
        if (node_count == MAX_NODES) {
            return -1;
        }
        for (int i = 0; i < MAX_NODES; i++) {
            if (tree[i].status == DONE) {
                LOCK(&tree[i].mutex);
                tree[i] = node;
                UNLOCK(&tree[i].mutex);
                node_count++;
                return i;
            }
        }
        return -1;
    }
};

SharedMem *shm;
int shmid, shmtreeid;

// Random tree generator
void genInitialTree() {
    int n = rand(MIN_INIT_NODES, MAX_INIT_NODES);
    for (int i = 0; i < n; i++) {
        Node node;
        int node_pos = shm->addNode(node);
        if (node_pos != -1) {
            if (shm->root == -1) {
                shm->root = node_pos;
                assert(shm->root == 0);
            } else {
                // Assign the parent of a node i as any node from 0 to i - 1
                int par = rand(0, node_pos - 1);
                while (shm->tree[par].child_count >= MAX_CHILD_JOBS) {
                    par = rand(0, node_pos - 1);
                }
                int status = shm->tree[par].addChild(node_pos);
                if (status != -1) {
                    shm->tree[node_pos].setParent(par);
                } else {
                    printf(COLOR_RED "Error: Failed to add child to parent\n" COLOR_RESET);
                }
            }
        } else {
            printf(COLOR_RED "Error: Failed to add node to tree\n" COLOR_RESET);
        }
    }
}

// Obtain a random job for the producer using DFS and a probability of selecting or rejecting a particular node
int getRandomJob(int start) {
    LOCK(&shm->tree[start].mutex);
    if (shm->tree[start].status != WAITING) {
        UNLOCK(&shm->tree[start].mutex);
        return -1;
    }
    double p = 0.3;
    double s = (double)rand() / RAND_MAX;

    // Suitable node found
    if (s < p && shm->tree[start].child_count < MAX_CHILD_JOBS) {
        return start;
    }
    UNLOCK(&shm->tree[start].mutex);
    for (int i = 0; i < MAX_CHILD_JOBS; i++) {
        int childstart = shm->tree[start].child_jobs[i];
        if (childstart != -1) {
            int id = getRandomJob(childstart);
            if (id != -1) {
                return id;
            }
        }
    }
    return -1;
}

// The function for the producer threads to execute
void *producer(void *arg) {
    srand(time(NULL) * gettid());
    int ind = *(int *)arg;
    int runtime = rand(MIN_PROD_TIME, MAX_PROD_TIME);
    printf(COLOR_GREEN "Producer %d started. Runtime %d seconds\n" COLOR_RESET, ind, runtime);
    auto start = chrono::high_resolution_clock::now();
    while (1) {
        auto curr = chrono::high_resolution_clock::now();
        auto diff = chrono::duration_cast<chrono::seconds>(curr - start);
        // Check if it has executed for the specified number of seconds
        if (diff.count() >= runtime) {
            printf(COLOR_GREEN "Producer %d finished\n" COLOR_RESET, ind);
            pthread_exit(NULL);
        }

        int par_pos = getRandomJob(shm->root);
        if (par_pos == -1) {
            usleep(5);
            continue;
        }
        Node node;
        node.setParent(par_pos);  // Set parent link
        LOCK(&shm->mutex);
        int child_pos = shm->addNode(node);  // Add a new node to the tree
        UNLOCK(&shm->mutex);
        if (child_pos == -1) {
            UNLOCK(&shm->tree[par_pos].mutex);
            usleep(5);
            continue;
        }
        int status = shm->tree[par_pos].addChild(child_pos);  // Set child link
        if (status != -1) {
            printf(COLOR_GREEN "Producer %d added child index %d to parent index %d. Job id: %d\n" COLOR_RESET, ind, child_pos, par_pos, node.job_id);
        }
        UNLOCK(&shm->tree[par_pos].mutex);

        int sleep_time = rand(MIN_PROD_SLEEP, MAX_PROD_SLEEP);
        usleep(sleep_time * 1000);
    }
}

// Obtain a job from a leaf of the tree for the consumer to be executed
int getLeaf(int start) {
    if (start == -1) {
        return -1;
    }
    LOCK(&shm->tree[start].mutex);
    // Suitable node found
    if (shm->tree[start].child_count == 0 && shm->tree[start].status == WAITING) {
        return start;
    } else {
        UNLOCK(&shm->tree[start].mutex);
        for (int i = 0; i < MAX_CHILD_JOBS; i++) {
            int childstart = shm->tree[start].child_jobs[i];
            if (childstart != -1) {
                int leaf_pos = getLeaf(childstart);
                if (leaf_pos != -1) {
                    return leaf_pos;
                }
            }
        }
        return -1;
    }
}

// The function for the consumer threads to execute
void *consumer(void *arg) {
    srand(time(NULL) * gettid());
    int ind = *(int *)arg;
    printf(COLOR_BLUE "Consumer %d started\n" COLOR_RESET, ind);
    while (1) {
        LOCK(&shm->tree[shm->root].mutex);
        // If the root is done, that implies all jobs are completed, hence we can exit from the comsumer thread
        if (shm->tree[shm->root].status != WAITING) {
            UNLOCK(&shm->tree[shm->root].mutex);
            printf(COLOR_BLUE "Consumer %d finished\n" COLOR_RESET, ind);
            pthread_exit(NULL);
        }
        UNLOCK(&shm->tree[shm->root].mutex);

        int leaf_pos = getLeaf(shm->root);
        if (leaf_pos == -1) {
            usleep(5);
            continue;
        }

        shm->tree[leaf_pos].status = ONGOING;  // Change status to ONGOING
        int par_pos = shm->tree[leaf_pos].parent;
        UNLOCK(&shm->tree[leaf_pos].mutex);
        printf(COLOR_BLUE "Job at index %d started by consumer %d. Job id: %d\n" COLOR_RESET, leaf_pos, ind, shm->tree[leaf_pos].job_id);
        usleep(shm->tree[leaf_pos].comp_time * 1000);  // Sleep for the required amount of time

        LOCK(&shm->mutex);
        LOCK(&shm->tree[leaf_pos].mutex);
        shm->tree[leaf_pos].status = DONE;  // Change status to DONE
        UNLOCK(&shm->tree[leaf_pos].mutex);
        shm->node_count--;
        printf(COLOR_BLUE "Job at index %d completed. Job id: %d\n" COLOR_RESET, leaf_pos, shm->tree[leaf_pos].job_id);
        UNLOCK(&shm->mutex);

        if (leaf_pos == shm->root) {  // root
            printf(COLOR_BLUE "Root finished\n" COLOR_RESET);
            continue;
        }

        LOCK(&shm->tree[par_pos].mutex);
        int status = shm->tree[par_pos].deleteChild(leaf_pos);  // Delete child link
        if (status == -1) {
            printf(COLOR_RED "Error: Failed to delete child from parent\n" COLOR_RESET);
        }
        UNLOCK(&shm->tree[par_pos].mutex);
    }
}

int main() {
    srand(time(NULL) * getpid());
    int np, nc;
    cout << "Enter no. of producer threads: ";
    cin >> np;
    cout << "Enter no. of consumer threads: ";
    cin >> nc;
    if (np < 0) {
        printf(COLOR_RED "No. of producer threads cannot be negative\n" COLOR_RESET);
        exit(1);
    }
    if (nc <= 0) {
        printf(COLOR_RED "No. of consumer threads should be atleast 1\n" COLOR_RESET);
        exit(1);
    }

    // Min sleep time for each producer between addition of jobs is 200 ms
    // So maximum no. of jobs 1 producer can add = 20s / 200ms = 100, hence np producers can add 100 * np jobs
    MAX_NODES = 100 * np + MAX_INIT_NODES;

    shmid = shmget(IPC_PRIVATE, sizeof(SharedMem), IPC_CREAT | 0666);
    shm = (SharedMem *)shmat(shmid, NULL, 0);
    shmtreeid = shmget(IPC_PRIVATE, MAX_NODES * sizeof(Node), IPC_CREAT | 0666);
    shm->tree = (Node *)shmat(shmtreeid, NULL, 0);
    shm->init();

    genInitialTree();

    // Display the initial tree
    for (int i = 0; i < shm->node_count; i++) {
        printf("Node %d", i);
        printf(" -> ");
        for (int j = 0; j < MAX_CHILD_JOBS; j++) {
            if (shm->tree[i].child_jobs[j] != -1) {
                printf("%d ", shm->tree[i].child_jobs[j]);
            }
        }
        printf("\n");
    }

    pthread_t prods[np];
    for (int i = 0; i < np; i++) {
        int *ind = new int(i);
        pthread_create(&prods[i], NULL, producer, ind);  // Create producer threads
    }

    pid_t b_pid = fork();
    if (b_pid == 0) {
        srand(time(NULL) * getpid());
        pthread_t cons[nc];
        for (int i = 0; i < nc; i++) {
            int *ind = new int(i);
            pthread_create(&cons[i], NULL, consumer, ind);  // Create consumer threads
        }
        // Wait for consumers to complete
        for (int i = 0; i < nc; i++) {
            pthread_join(cons[i], NULL);
        }
        exit(0);
    } else if (b_pid < 0) {
        perror("fork");
        exit(1);
    }

    // Ensure all producers have stopped
    for (int i = 0; i < np; i++) {
        pthread_join(prods[i], NULL);
    }

    // Wait for the process B to complete
    waitpid(b_pid, NULL, 0);

    // Release mutex locks
    for (int i = 0; i < MAX_NODES; i++) {
        pthread_mutex_destroy(&shm->tree[i].mutex);
    }
    pthread_mutex_destroy(&shm->mutex);

    // Release shared memory
    shmdt(shm->tree);
    shmdt(shm);
    shmctl(shmtreeid, IPC_RMID, NULL);
    shmctl(shmid, IPC_RMID, NULL);
}
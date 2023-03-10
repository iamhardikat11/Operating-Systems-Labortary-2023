#include <iostream>
#include <queue>
#include <pthread.h>
#include <unistd.h>

using namespace std;

// Global Variables
pthread_mutex_t actionQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t postQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t actionQueueNotEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t postQueueNotEmpty = PTHREAD_COND_INITIALIZER;
queue<string> actionQueue;
queue<string> postQueue;

// User Simulator Thread
void* userSimulator(void* arg) {
    while (true) {
        // create a new action and push it to the action queue
        string action = "new action";
        pthread_mutex_lock(&actionQueueMutex);
        actionQueue.push(action);
        cout << "User Simulator: " << action << " added to action queue" << endl;
        pthread_cond_signal(&actionQueueNotEmpty);
        pthread_mutex_unlock(&actionQueueMutex);
        // wait for some time before creating the next action
        sleep(1);
    }
    return NULL;
}

// Push Update Threads
void* pushUpdate(void* arg) {
    int threadId = *(int*)arg;
    while (true) {
        // pop an action from the action queue and add it to the post queue and the neighbor of the node
        pthread_mutex_lock(&actionQueueMutex);
        while (actionQueue.empty()) {
            pthread_cond_wait(&actionQueueNotEmpty, &actionQueueMutex);
        }
        string action = actionQueue.front();
        actionQueue.pop();
        pthread_mutex_unlock(&actionQueueMutex);
        // add the action to the post queue
        pthread_mutex_lock(&postQueueMutex);
        postQueue.push("post from thread " + to_string(threadId) + ": " + action);
        cout << "Push Update Thread " << threadId << ": " << action << " added to post queue" << endl;
        pthread_cond_signal(&postQueueNotEmpty);
        pthread_mutex_unlock(&postQueueMutex);
    }
    return NULL;
}

// Read Post Threads
void* readPost(void* arg) {
    int threadId = *(int*)arg;
    while (true) {
        // pop a post from the post queue and print it
        pthread_mutex_lock(&postQueueMutex);
        while (postQueue.empty()) {
            pthread_cond_wait(&postQueueNotEmpty, &postQueueMutex);
        }
        string post = postQueue.front();
        postQueue.pop();
        pthread_mutex_unlock(&postQueueMutex);
        // print the post
        cout << "Read Post Thread " << threadId << ": " << post << endl;
    }
    return NULL;
}

// Main Function
int main() {
    // create user simulator thread
    pthread_t userSimulatorThread;
    pthread_create(&userSimulatorThread, NULL, userSimulator, NULL);

    // create push update threads
    const int NUM_THREADS = 25;
    pthread_t pushUpdateThreads[NUM_THREADS];
    int threadIds[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threadIds[i] = i;
        pthread_create(&pushUpdateThreads[i], NULL, pushUpdate, &threadIds[i]);
    }

    // create read post threads
    const int NUM_READ_THREADS = 10;
    pthread_t readPostThreads[NUM_READ_THREADS];
    int readThreadIds[NUM_READ_THREADS];
    for (int i = 0; i < NUM_READ_THREADS; i++) {
        readThreadIds[i] = i;
        pthread_create(&readPostThreads[i], NULL, readPost, &readThreadIds[i]);
    }
    pthread_join(userSimulatorThread, NULL);
    // wait for threads to
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(pushUpdateThreads[i], NULL);
    }
    for (int i = 0; i < NUM_READ_THREADS; i++) {
        pthread_join(readPostThreads[i], NULL);
    }
    pthread_exit(NULL);
}
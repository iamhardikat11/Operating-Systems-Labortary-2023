#include <bits/stdc++.h>
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

// Returns a random number between low and high
int rand(int low, int high)
{
    return low + rand() % (high - low + 1);
}

// A wrapper around pthread_mutex_lock for error detection
void LOCK(pthread_mutex_t *mutex)
{
    int status = pthread_mutex_lock(mutex);
    if (status != 0)
    {
        printf(COLOR_RED "pthread_mutex_lock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

// A wrapper around pthread_mutex_unlock for error detection
void UNLOCK(pthread_mutex_t *mutex)
{
    int status = pthread_mutex_unlock(mutex);
    if (status != 0)
    {
        printf(COLOR_RED "pthread_mutex_unlock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

typedef struct
{
    int user_id;        // It is just the node_id for the node
    int action_id;      // Additional Information about 4th like, 5th post
    string action_type; // one of "post, comment, like"
    time_t timestamp;   // Simple Unix/Linux Timestamp

    void Action(int user_id, int action_id, string action_type)
    {
        this->user_id = user_id;
        this->action_id = action_id;
        this->action_type = action_type;
    }
} Action;

typedef struct
{
    int cnt_post;
    int cnt_comment;
    int cnt_like;
    deque<Action> actl;
    void ActionQueue()
    {
        this->cnt_comment = 0;
        this->cnt_like = 0;
        this->cnt_post = 0;
        actl.clear();
    }
} ActionQueue;

typedef struct
{
    int id; // node id
    int degree;
    int order; // Encoded as 0 <- priority and 1 <- chronological (To be Set at Random)
    map<int,int> neighbours; // Hashmap to store the neighbours of the given Node
    ActionQueue Wall;  // Action by U itself
    ActionQueue Feed; //  Action by All it's Neighbours
    void init()
    {
        this->id = 0;
        this->degree = 0;
        this->neighbours.clear();
        this->order = rand()%2;
        this->Wall.ActionQueue();
        this->Feed.ActionQueue();
    }
    void addEdge(int id, int neigh_id)
    {
        this->id = id;
        this->degree++;
        this->neighbours[neigh_id] = 1;
    }
} Node;

signed main()
{
    
}
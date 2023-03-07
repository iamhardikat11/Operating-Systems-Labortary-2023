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
#include <pthread.h>
#include <chrono>
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

string filename = "musae_git_edges.csv";

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
// Struct to hold the argument
struct ThreadArgs {
    chrono::high_resolution_clock::time_point start_time;
};

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
        time(&this->timestamp);
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
    int order;                // Encoded as 0 <- priority and 1 <- chronological (To be Set at Random)
    map<int, int> neighbours; // Hashmap to store the neighbours of the given Node
    ActionQueue Wall;         // Action by U itself
    ActionQueue Feed;         //  Action by All it's Neighbours
    void init(int id)
    {
        this->id = id;
        this->degree = 0;
        this->neighbours.clear();
        this->order = rand() % 2;
        this->Wall.ActionQueue();
        this->Feed.ActionQueue();
    }
    void addEdge(int neigh_id)
    {
        this->degree++;
        this->neighbours[neigh_id] = 1;
    }
    void addWallQueue(string action_type)
    {
        Action aq;
        int q = 0;
        if (action_type == "post")
        {
            q = Wall.cnt_post + 1;
        }
        else if (action_type == "comment")
        {
            q = Wall.cnt_comment + 1;
        }
        else if (action_type == "like")
        {
            q = Wall.cnt_like + 1;
        }
        else
        {
            perror("The Action is NOT a comment, post, or like\n");
            exit(1);
        }
        aq.Action(this->id, q, action_type);
        this->Wall.actl.push_back(aq);
    }
    void addFeedQueue(string action_type, int action_id)
    {
        Action aq;
        if (action_type != "post" && action_type != "comment" && action_type != "like")
        {
            perror("The Action is NOT a comment, post, or like\n");
            exit(1);
        }
        else if (action_type == "post")
            this->Feed.cnt_post++;
        else if (action_type == "comment")
            this->Feed.cnt_comment++;
        else
            this->Feed.cnt_like++;
        aq.Action(this->id, action_id, action_type);
        this->Feed.actl.push_back(aq);
    }
} Node;


//  create map of nodes
//  create map of all action queues, from where to pop in pushUpdates

ActionQueue AQueue;
map<int, set<int>> test;
map<int, Node> graph;

// The function for the producer threads to execute
void *userSimulator(void *arg) {
    ThreadArgs* thread_args = (ThreadArgs*)arg;
    chrono::high_resolution_clock::time_point start = thread_args->start_time;
    srand(time(NULL) * getpid());
    auto curr = chrono::high_resolution_clock::now();
    printf(COLOR_GREEN "UserSimulator started. Runtime %lld seconds\n" COLOR_RESET, chrono::duration_cast<chrono::seconds>(curr - start));
    int i = 1;
    vector<string> action = {"post", "comment", "like"};
    while (1) {
        curr = chrono::high_resolution_clock::now();
        printf(COLOR_RED "Iteration #%d. Runtime %lld seconds\n" COLOR_RESET, i, chrono::duration_cast<chrono::seconds>(curr - start));
        int NUM_NODE = graph.size();
        auto curr = chrono::high_resolution_clock::now();
        auto diff = chrono::duration_cast<chrono::seconds>(curr - start);
        vector<int> user_ids;
        for(int i=0;i<100;i++) user_ids.push_back(rand()%(NUM_NODE-1)+1);
        for(auto user_id: user_ids) 
        {
            int action_created = log2(graph[user_id].degree);
            for(int j=0;j<action_created;j++)
            {
                Action aq;
            }
            cout << user_id << " ";
        }
        cout << endl;
        sleep(120);
        i++;
        // Check if it has executed for the specified number of seconds
        // LOCK(&shm->mutex);
        // int child_pos = shm->addNode(node);  // Add a new node to the tree
        // UNLOCK(&shm->mutex);
        // if (child_pos == -1) {
        //     UNLOCK(&shm->tree[par_pos].mutex);
        //     usleep(5);
        //     continue;
        // }
        // int status = shm->tree[par_pos].addChild(child_pos);  // Set child link
        // if (status != -1) {
        //     printf(COLOR_GREEN "Producer %d added child index %d to parent index %d. Job id: %d\n" COLOR_RESET, ind, child_pos, par_pos, node.job_id);
        // }
        // UNLOCK(&shm->tree[par_pos].mutex);
    }
    pthread_exit(NULL);
}

void *pushUpdates(void *data)
{
    Action A = AQueue.actl.front();
    AQueue.actl.pop_front();
    if (A.action_type == "post")
        AQueue.cnt_post--;
    else if (A.action_type == "comment")
        AQueue.cnt_comment--;
    else
        AQueue.cnt_like--;
    // Push the Action to the Wall of the User
    Node n = graph[A.user_id];
    n.Wall.actl.push_back(A);
    if (A.action_type == "post")
        n.Wall.cnt_post++;
    else if (A.action_type == "comment")
        n.Wall.cnt_comment++;
    else
        n.Wall.cnt_like++;
    // Push the Action to the Feed of the User's Neighbours
    for (auto it = n.neighbours.begin(); it != n.neighbours.end(); it++)
    {
        Node n1 = graph[it->first];
        n1.Feed.actl.push_back(A);
        if (A.action_type == "post")
            n1.Feed.cnt_post++;
        else if (A.action_type == "comment")
            n1.Feed.cnt_comment++;
        else
            n1.Feed.cnt_like++;
    }
    pthread_exit(NULL);
}

signed main()
{
    // Starting Time of Execution
    auto start = chrono::high_resolution_clock::now();
    // Open the CSV file for reading
    ifstream infile(filename);
    if(!infile)
    {
        cerr << "Unable to open " << filename << endl;
        exit(EXIT_FAILURE);
    }
    cout << COLOR_BLUE << "----PUSH_UPDATE SYSTEM----" << COLOR_RESET << endl;
    // Read the CSV file line by line
    string line;
    int i = 0, id1, id2;
    while (getline(infile, line))
    {
        // Split the line by commas
        if (i == 0)
        {
            i++;
            continue;
        }
        sscanf(line.c_str(), "%d,%d", &id1, &id2);
        i++;
        // Add the ids to the vector
        Node n1, n2;
        if (graph.count(id1) == 0)
        {
            n1.init(id1);
            graph[id1] = n1;
        }
        else
            n1 = graph[id1];
        if (graph.count(id2) == 0)
        {
            n2.init(id2);
            graph[id2] = n2;
        }
        else
            n2 = graph[id2];
        n1.addEdge(id2);
        graph[id1] = n1;
        n2.addEdge(id1);
        graph[id2] = n2;
        test[id1].insert(id2);
        test[id2].insert(id1);
    }
    infile.close();
#ifdef DEBUG_LOAD
    std::ofstream file1("output_graph.txt");
    if(!file1)
    {
        cerr << "Unable to open " << "output_graph.txt" << endl;
        exit(EXIT_FAILURE);
    }
    for (auto it : graph)
    {
        file1 << it.first << ": " << it.second.neighbours.size() << " ";
        for (auto x : it.second.neighbours)
        {
            file1 << x.first << " ";
        }
        file1 << endl;
    }
    file1.close();
    std::ofstream file2("output_test.txt");
    if(!file2)
    {
        cerr << "Unable to open " << "output_test.txt" << endl;
        exit(EXIT_FAILURE);
    }
    for (auto it : test)
    {
        file2 << it.first << ": " << it.second.size() << " ";
        for (auto x : it.second)
        {
            file2 << x << " ";
        }
        file2 << endl;
    }
    file2.close();
#endif
// UserSimulator Thread
    pthread_t user_simulator;
    ThreadArgs thread_args;
    thread_args.start_time = start;
    printf("In main: Creating UserSimulator Thread\n");
    int ret = pthread_create(&user_simulator, NULL, userSimulator, (void*)&thread_args);
    if (ret != 0)
    {
        printf("Error: pthread_create() failed\n");
        exit(EXIT_FAILURE);
    }
    pthread_join(user_simulator, NULL);
    pthread_cancel(user_simulator);
    pthread_join(user_simulator, NULL);
// PushUpdate Threads
    // pthread_t push_updates[25];
    // for (int i = 0; i < 25; i++)
    // {
    //     ret = pthread_create(&push_updates[i], NULL, pushUpdates, (void *)i);
    //     if (ret != 0)
    //     {
    //         printf("Error: pthread_create() failed\n");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // // join all the threads
    // for (int i = 0; i < 25; i++)
    //     pthread_join(push_updates[i], NULL);

    pthread_exit(NULL);
}
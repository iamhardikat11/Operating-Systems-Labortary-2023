#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <string.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include <queue>
#include <chrono>
#include <thread>
using namespace std;

#define USER_SIMULATOR 1
#define PUSH_UPDATE 25
#define READ_POST 10
#define QUEUE_SIZE 150

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"

string filename = "musae_git_edges.csv";
string output_file = "sns.log";
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condWall = PTHREAD_COND_INITIALIZER;
pthread_cond_t condFeed = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSleeep = PTHREAD_COND_INITIALIZER;
// Returns a random number between low and high
int rand(int low, int high)
{
    return low + rand() % (high - low + 1);
}

// A wrapper around pthread_mutex_lock for error detection
void LOCK(pthread_mutex_t mutex)
{
    int status = pthread_mutex_lock(&mutex);
    if (status != 0)
    {
        printf(COLOR_RED "pthread_mutex_lock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

// A wrapper around pthread_mutex_unlock for error detection
void UNLOCK(pthread_mutex_t mutex)
{
    int status = pthread_mutex_unlock(&mutex);
    if (status != 0)
    {
        printf(COLOR_RED "pthread_mutex_unlock failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}
// Struct to hold the argument
struct ThreadArgs
{
    chrono::high_resolution_clock::time_point start_time;
    pthread_mutex_t mutex;
};

typedef struct Action
{
    int user_id;        // It is just the node_id for the node
    int action_id;      // Additional Information about 4th like, 5th post
    string action_type; // one of "post, comment, like"
    time_t timestamp;   // Simple Unix/Linux Timestamp
    int reader_id;

    void init(int user_id, int action_id, string action_type)
    {
        this->user_id = user_id;
        this->action_id = action_id;
        this->action_type = action_type;
        this->reader_id = -1;
        time(&this->timestamp);
    }
    void add_Reader(int reader_id)
    {
        this->reader_id = reader_id;
    }
    Action &operator=(const Action &other)
    {
        if (this != &other)
        { // check for self-assignment

            this->user_id = other.user_id;
            this->action_id = other.action_id;
            this->action_type = other.action_type;
            this->timestamp = other.timestamp;
            this->reader_id = other.reader_id;
        }
        return *this;
    }
} Action;

typedef struct ActionQueue
{
    int cnt_post;
    int cnt_comment;
    int cnt_like;
    deque<Action> actl;
    void init()
    {
        this->cnt_comment = 0;
        this->cnt_like = 0;
        this->cnt_post = 0;
        actl.clear();
    }
    void addAction(Action A)
    {
        if (A.action_type == "post")
            this->cnt_post--;
        else if (A.action_type == "comment")
            this->cnt_comment--;
        else if (A.action_type == "like")
            this->cnt_like--;
        else
        {
            perror("action_error: No Such Action Exists.\n");
            exit(0);
        }
        actl.push_back(A);
    }
    Action pop()
    {
        if (this->actl.empty())
        {
            perror("queue_error: Queue Empty\n");
            exit(1);
        }
        Action a = this->actl.front();
        this->actl.pop_front();
        return a;
    }
} ActionQueue;

typedef struct Node
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
        this->Wall.init();
        this->Feed.init();
    }
    void addEdge(int neigh_id)
    {
        this->degree++;
        this->neighbours[neigh_id] = 1;
    }
    int addWallQueue(string action_type)
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
        aq.init(this->id, q, action_type);
        this->Wall.actl.push_back(aq);
        return q;
    }
    void addFeedQueue(int user_id, string action_type, int action_id)
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
        aq.init(user_id, action_id, action_type);
        aq.add_Reader(this->id);
        this->Feed.actl.push_back(aq);
    }
    void AddFeedAction(Action A)
    {
        if (A.action_type == "post")
            this->Feed.cnt_post++;
        else if (A.action_type == "comment")
            this->Feed.cnt_comment++;
        else if (A.action_type == "like")
            this->Feed.cnt_like++;
        else
        {
            perror("action_error: No Such Action Exists.\n");
            exit(0);
        }
        this->Feed.actl.push_back(A);
    }
    Node &operator=(const Node &other)
    {
        if (this != &other)
        { // check for self-assignment

            this->id = other.id;
            this->degree = other.degree;
            this->order = other.order;
            this->neighbours = other.neighbours;
            this->Wall = other.Wall;
            this->Feed = other.Feed;
        }
        return *this;
    }
} Node;

//  create map of nodes
//  create map of all action queues, from where to pop in pushUpdates

struct cmp
{
    bool operator()(const std::pair<int, int>& a, const std::pair<int, int>& b) const { return a.second > b.second; }
};
ActionQueue AQueue;
map<int, set<int>> test;
std::map<int, std::map<int, int>> priority_map;
map<int, Node> graph;
map<int, int> mp;
map<int, int> mp1;
int sleep_flag = 0;

/*
* implementation of the priority map
*/
void precomutePriority()
{
    for (auto const &node : graph)
    {
        map<int, int> neighbor_counts;
        for (auto const &neighbor_id : node.second.neighbours)
        {
            for (auto const &neighbor_of_neighbor_id : graph[neighbor_id.first].neighbours)
            {
                if (neighbor_of_neighbor_id.first != node.first)
                {
                    priority_map[node.first][neighbor_of_neighbor_id.first]++;
                }
            }
        }
    }
}
// The function for the producer threads to execute
void *userSimulator(void *arg)
{
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    chrono::high_resolution_clock::time_point start = thread_args->start_time;
    srand(time(NULL) * getpid());
    std::ofstream file(output_file, std::ios::app);
    if (!file.is_open())
    {
        cerr << "Unable to open "
             << output_file << endl;
        exit(EXIT_FAILURE);
    }
    auto curr = chrono::high_resolution_clock::now();
    printf(COLOR_GREEN "UserSimulator started. Runtime %lld seconds\n" COLOR_RESET, chrono::duration_cast<chrono::seconds>(curr - start));
    file << "UserSimulator started. Runtime " << chrono::duration_cast<chrono::seconds>(curr - start).count() << " seconds\n";
    file.close();
    int i = 1;
    vector<string> action = {"post", "comment", "like"};
    for (int x = 1;; x++)
    {
        std::ofstream outfile(output_file, std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open "
                 << output_file << endl;
            exit(EXIT_FAILURE);
        }
        curr = chrono::high_resolution_clock::now();
        printf(COLOR_RED "Iteration #%d. Runtime %lld seconds\n" COLOR_RESET, x, chrono::duration_cast<chrono::seconds>(curr - start));
        outfile << "Iteration #" << i << ". Runtime " << (chrono::duration_cast<chrono::seconds>(curr - start).count()) << " seconds\n";
        int NUM_NODE = graph.size();
        auto curr = chrono::high_resolution_clock::now();
        auto diff = chrono::duration_cast<chrono::seconds>(curr - start);
        printf("Nodes Selected: The following randomly selected Users will Create Actions.\n");
        outfile << "Nodes Selected: The following randomly selected Users will Create actions.\n";
        LOCK(mutex);
        while (AQueue.actl.size() == QUEUE_SIZE)
        {
            pthread_cond_wait(&condWall, &mutex);
        }
        for (int i = 0; i < 100; i++)
        {
            int user_id = (rand() % (NUM_NODE));
            int action_created = log2(graph[user_id].degree);
            mp[user_id] += action_created;
            for (int j = 0; j < action_created; j++)
            {
                Node n = graph[user_id];
                string action_type = action[rand() % 3];
                int action_id = n.addWallQueue(action_type);
                Action ac;
                ac.init(user_id, action_id, action_type);
                AQueue.addAction(ac);
                graph[user_id] = n;
            }
            cout << user_id << " ";
            outfile << user_id << " ";
            if ((i + 1) % 25 == 0)
            {
                outfile << endl;
                cout << endl;
            }
        }
        if (outfile.is_open())
            outfile.close();
        sleep_flag = 1;
        pthread_cond_broadcast(&condWall);
        UNLOCK(mutex);
        sleep(3);
        // std::this_thread::sleep_for(std::chrono::minutes(2));
        sleep_flag = 0;
        pthread_cond_broadcast(&condSleeep);
        i++;
    }
    pthread_exit(NULL);
}

void *pushUpdate(void *arg)
{
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    chrono::high_resolution_clock::time_point start = thread_args->start_time;
    int i = 1;
    while (1)
    {
        // LOCK(mutex);
        queue<pair<int, int>> message;
        std::ofstream outfile(output_file, std::ios::app);
        if (!outfile.is_open())
        {
            cerr << "Unable to open "
                 << output_file << endl;
            exit(EXIT_FAILURE);
        }
        outfile.close();
        while (AQueue.actl.empty())
            pthread_cond_wait(&condWall, &mutex);
        while (!AQueue.actl.empty())
        {
            cout << AQueue.actl.size() << endl;
            // LOCK(mutex);
            Action A = AQueue.pop();
            if (A.action_type == "post")
                AQueue.cnt_post--;
            else if (A.action_type == "comment")
                AQueue.cnt_comment--;
            else if (A.action_type == "like")
                AQueue.cnt_like--;
            else
            {
                perror("action_error: No Such Action Exists.\n");
                exit(0);
            }
            // cout << AQueue.cnt_post << " " << AQueue.cnt_comment << " " << AQueue.cnt_like << endl;
            // Push the Action to the Wall of the User
            Node n = graph[A.user_id];
            // Push the Action to the Feed of the User's Neighbours
            for (auto it : n.neighbours)
            {
                A.add_Reader(it.first);
                graph[it.first].AddFeedAction(A);
                std::ofstream outfile(output_file, std::ios::app);
                if (!outfile.is_open())
                {
                    cerr << "Unable to open "
                         << output_file << endl;
                    exit(EXIT_FAILURE);
                }
                cout << "Pushing to " << A.reader_id << " "
                     << "from " << A.user_id << endl;
                outfile << "Pushing to " << A.reader_id << " "
                        << "from " << A.user_id << endl;
                if (outfile.is_open())
                    outfile.close();
                mp1[it.first] = 1;
            }
            // if (sleep_flag)
            // {
            //     while(sleep_flag && !message.empty())
            //     {
            //     }
            // }
            i++;
        }
        pthread_cond_broadcast(&condWall);
        pthread_cond_broadcast(&condFeed);
        // UNLOCK(mutex);
        // UNLOCK(mutex);
    }
    pthread_exit(NULL);
}

// void *readPost(void *arg)
// {
//     ThreadArgs *thread_args = (ThreadArgs *)arg;
//     chrono::high_resolution_clock::time_point start = thread_args->start_time;
//     pthread_mutex_t mutex = thread_args->mutex;
//     while (1)
//     {
//         LOCK(mutex);
//         while (AQueue.actl.empty())
//             pthread_cond_wait(&condWall, &mutex);
//         while (!AQueue.actl.empty())
//         {
//             Action A = AQueue.actl.front();
//             AQueue.actl.pop_front();
//             if (A.action_type == "post")
//                 AQueue.cnt_post--;
//             else if (A.action_type == "comment")
//                 AQueue.cnt_comment--;
//             else
//                 AQueue.cnt_like--;
//             // Push the Action to the Wall of the User
//             Node n = graph[A.user_id];
//             // Push the Action to the Feed of the User's Neighbours
//             for (auto it : n.neighbours)
//             {
//                 Node n1 = graph[it.first];
//                 A.reader_id = it.first;
//                 n1.AddFeedAction(A);
//                 n1.Feed.actl.push_back(A);
//                 graph[it.first] = n1;
//                 mp1[it.first] = 1;

//             }
//         }
//         UNLOCK(mutex);
//     }
// }
signed main()
{
    std::ofstream file_clear(output_file);
    if (!file_clear.is_open())
    {
        cerr << "Unable to open "
             << output_file << endl;
        exit(EXIT_FAILURE);
    }
    if (file_clear.is_open())
        file_clear.close();
    test.clear();
    graph.clear();
    mp.clear();
    mp1.clear();
    priority_map.clear();
    precomutePriority();
#ifdef DEBUG_PRIORITY
    ofstream file(filename);
    if (!file)
    {
        cerr << "Unable to open " << filename << endl;
        exit(EXIT_FAILURE);
    }
    for(auto it: priority_map)
    {
        for(auto x: it.second)
        {
            cout << it.first << " " << x.first << " " << x.second << endl;
        }
    }
    if(file.is_open())
        file.close();
#endif
    void *status;
    // Starting Time of Execution
    auto start = chrono::high_resolution_clock::now();
    // Open the CSV file for reading
    ifstream infile(filename);
    if (!infile)
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
    if (infile.is_open())
        infile.close();
#ifdef DEBUG_LOAD
    std::ofstream file("degree.txt", std::ios::app);
    if (!file)
    {
        cerr << "Unable to open "
             << "degree.txt" << endl;
        exit(EXIT_FAILURE);
    }
    for (auto it : graph)
    {
        file << it.first << ": " << it.second.degree << endl;
    }
    if (file.is_open())
        file.close();
    std::ofstream file1("output_graph.txt", std::ios::app);
    if (!file1)
    {
        cerr << "Unable to open "
             << "output_graph.txt" << endl;
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
    if (file1.is_open())
        file1.close();
    std::ofstream file2("output_test.txt", std::ios::app);
    if (!file2)
    {
        cerr << "Unable to open "
             << "output_test.txt" << endl;
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
    if (file2.is_open())
        file2.close();
#endif
    AQueue.init();
    // UserSimulator Thread
    pthread_t user_simulator[USER_SIMULATOR];
    ThreadArgs thread_args;
    thread_args.start_time = start;
    printf("In main: Creating UserSimulator Thread\n");
    int ret = 0;
    // mtx.lock();
    for (int i = 0; i < USER_SIMULATOR; i++)
    {
        ret = pthread_create(&user_simulator[i], NULL, userSimulator, (void *)&thread_args);
        if (ret != 0)
        {
            perror("Error: pthread_create() failed\n");
            exit(EXIT_FAILURE);
        }
    }

    // PushUpdate Threads
    pthread_t push_updates[PUSH_UPDATE];
    // mtx.lock();
    for (int i = 0; i < PUSH_UPDATE; i++)
    {
        ret = pthread_create(&push_updates[i], NULL, pushUpdate, (void *)&thread_args);
        if (ret != 0)
        {
            printf("Error: pthread_create() failed\n");
            exit(EXIT_FAILURE);
        }
    }
    int cnt = 0;
    for (int i = 0; i < USER_SIMULATOR; i++)
    {
        ret = pthread_join(user_simulator[i], &status);
        cnt++;
        if (ret != 0)
        {
            perror("Error: pthread_join() failed\n");
            exit(EXIT_FAILURE);
        }
    }
    // join all the threads
    for (int i = 0; i < PUSH_UPDATE; i++)
    {
        ret = pthread_join(push_updates[i], &status);
        cnt++;
        if (ret != 0)
        {
            perror("Error: pthread_join() failed\n");
            exit(EXIT_FAILURE);
        }
    }
#ifdef DEBUG_SUM
    int ans = 0;
    for (auto it : mp)
    {
        cout << it.first << " " << graph[it.first].Feed.actl.size() << " " << graph[it.first].Wall.actl.size() << endl;
        ans += graph[it.first].Wall.actl.size();
    }
    cout << ans << " " << AQueue.actl.size() << endl;
#endif
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condWall);
    pthread_cond_destroy(&condFeed);
    pthread_cond_destroy(&condSleeep);
    cout << cnt << endl;
    pthread_exit(NULL);
}
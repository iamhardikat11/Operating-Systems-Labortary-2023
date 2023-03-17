// g++ -pthread -std=c++11 -o a.out main.cpp
#include <bits/stdc++.h>
#include <string.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

using namespace __gnu_pbds;
using namespace std;

#define USER_SIMULATOR 1
#define PUSH_UPDATE 25
#define READ_POST 10
#define QUEUE_SIZE 150

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"
#define NUM_NODES 37700

auto start = chrono::high_resolution_clock::now();
string filename = "musae_git_edges.csv";
string output_file = "sns.log";
#define total_users 37700
#define total_edges 289003
char *buff = (char *)malloc(100);
#define ordered_set tree<int, null_type, less<int>, rb_tree_tag, tree_order_statistics_node_update> //

// A Function to print the Time Elapsed
void printTime(chrono::high_resolution_clock::time_point start, chrono::high_resolution_clock::time_point curr)
{
    cout << "  Time Elapsed:- " << (chrono::duration_cast<chrono::seconds>(curr - start).count()) << " seconds\n";
}

// Struct to hold the argument
struct ThreadArgs
{
    chrono::high_resolution_clock::time_point start_time;
    pthread_mutex_t mutex;
    int thread_id;
};

typedef struct Action
{
    int user_id;        // It is just the node_id for the node
    int action_id;      // Additional Information about 4th like, 5th post
    string action_type; // one of "post, comment, like"
    time_t timestamp;   // Simple Unix/Linux Timestamp
    int order;
    int reader_id;

    void init(int user_id, int action_id, string action_type, int order)
    {
        this->user_id = user_id;
        this->action_id = action_id;
        this->action_type = action_type;
        this->reader_id = -1;
        this->order = order;
        time(&this->timestamp);
    }
    void add_Reader(int reader_id)
    {
        this->reader_id = reader_id;
    }
    Action &operator=(const Action &other)
    {
        { // check for self-assignment

            this->user_id = other.user_id;
            this->action_id = other.action_id;
            this->action_type = other.action_type;
            this->timestamp = other.timestamp;
            this->reader_id = other.reader_id;
            this->order = order;
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
            Action ac;
            ac.init(-1, -1, "Continue", 0);
            return ac;
        }
        Action a = this->actl.front();
        this->actl.pop_front();
        return a;
    }
} ActionQueue;

typedef struct Node1
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
    int addWallQueue(string action_type, int order)
    {
        Action aq;
        int q = 0;
        if (action_type == "post")
        {
            q = Wall.cnt_post + 1;
            Wall.cnt_post++;
        }
        else if (action_type == "comment")
        {
            q = Wall.cnt_comment + 1;
            Wall.cnt_comment++;
        }
        else if (action_type == "like")
        {
            q = Wall.cnt_like + 1;
            Wall.cnt_like++;
        }
        else
        {
            perror("The Action is NOT a comment, post, or like\n");
            exit(1);
        }
        aq.init(this->id, q, action_type, order);
        this->Wall.actl.push_back(aq);
        return q;
    }
    void addFeedQueue(int user_id, string action_type, int action_id, int order)
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
        aq.init(user_id, action_id, action_type, order);
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
    Node1 &operator=(const Node1 &other)
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
} Node1;


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

// A wrapper around UNLOCK for error detection
void UNLOCK(pthread_mutex_t mutex)
{
    int status = pthread_mutex_unlock(&mutex);
    if (status != 0)
    {
        printf(COLOR_RED "UNLOCK failed: %s\n" COLOR_RESET, strerror(status));
        exit(1);
    }
}

// struct action
struct action
{
    int user_id;
    int action_id;
    int action_type;
    time_t time_stamp;
    int priority;
};

struct cmp1 // comparator for priority queue
{
    bool operator()(action a, action b)
    {
        return a.time_stamp > b.time_stamp;
    }
};

struct cmp2 //
{
    bool operator()(action a, action b)
    {
        return a.priority > b.priority;
    }
};

// Node class for user
struct Node
{
public:
    int id;
    int action_cnt[3];
    int order;                                        // 0 for priority and 1 for chronological
    queue<action> wq;                                 // wall queue for user
    priority_queue<action, vector<action>, cmp1> fq1; // feed queue for user
    priority_queue<action, vector<action>, cmp2> fq2; // feed queue for user
    Node()
    {
        this->order = rand() % 2;
        this->action_cnt[0] = 0;
        this->action_cnt[1] = 0;
        this->action_cnt[2] = 0;
        fq1 = priority_queue<action, vector<action>, cmp1>();
        fq2 = priority_queue<action, vector<action>, cmp2>();
    }
};
map<int, set<int>> graph;
map<pair<int, int>, int> neighbours;
vector<Node> User(total_users, Node());
map<int, int> degree;
map<int, string> actions_types;

queue<action> shared_queue;
pthread_mutex_t shared_queue_lock;
pthread_cond_t shared_queue_cond;
int shared_queue_size = 0;

// ordered_set cfq;
queue<int> cfq;
pthread_mutex_t cfq_lock;
pthread_cond_t cfq_cond;
int cfq_size = 0;

pthread_mutex_t feed_queue_lock[total_users];
int feed_queue_size[total_users];
// pthread_cond_t feed_queue_cond;

FILE *outputfile = fopen(output_file.c_str(), "wb");

void PRINT(char *buff)
{
    int buff_size = sizeof(char) * strlen(buff);
    fwrite(buff, sizeof(char), buff_size, outputfile);
    fwrite(buff, sizeof(char), buff_size, stdout);
}
void calculate_common_neigbours(int u, int v)
{
    int cnt = 0;
    for (auto it : graph[u])
        if (graph[v].find(it) != graph[v].end())
            cnt++;
    neighbours[make_pair(u, v)] = cnt;
    neighbours[make_pair(v, u)] = cnt;
}

void *userSimulator(void *arg)
{
    memset(buff, 0, 100);
    auto curr = chrono::high_resolution_clock::now();
    printf(COLOR_GREEN "UserSimulator started. Runtime %lld seconds\n" COLOR_RESET, chrono::duration_cast<chrono::seconds>(curr - start));
    for (int x = 0;; x++)
    {
        set<int> nodes; // set of nodes to be selected
        for (; nodes.size() < 100;)
        {
            int random_node = rand() % total_users;
            sprintf(buff, "Selected Node: %d \n", random_node);
            PRINT(buff);
            nodes.insert(random_node);
        }
        for (auto it : nodes)
        {
            PRINT(buff);
            if (degree[it] == 0)
                continue;
            int n = log2(degree[it]) + 1;
            for (int i = 0; i < n; i++)
            {
                int r = rand() % 3;
                action act;
                act.action_id = User[it].action_cnt[r] + 1;
                User[it].action_cnt[r] += 1;
                act.action_type = r;
                act.time_stamp = time(NULL); // time stamp
                act.user_id = it;
                User[it].wq.push(act);

                // Critical Section Start
                LOCK(shared_queue_lock);

                shared_queue.push(act);
                shared_queue_size++;
                pthread_cond_broadcast(&shared_queue_cond);

                UNLOCK(shared_queue_lock);

                // Critical Section End

                sprintf(buff, "User %d take action : %s %d on %s", it, actions_types[r].c_str(), act.action_id, ctime(&act.time_stamp));
                PRINT(buff);
            }
            for (auto itr : graph[it])
            {
                calculate_common_neigbours(it, itr);
            };
        }
        std::this_thread::sleep_for(std::chrono::seconds(40));
    }
    pthread_exit(NULL);
}

void *pushUpdate(void *arg) // push update to feed queue
{
    memset(buff, 0, 100);
    while (1)
    {
        // Critical Section Start
        LOCK(shared_queue_lock);
        while (shared_queue_size == 0)
            pthread_cond_wait(&shared_queue_cond, &shared_queue_lock);
        action act = shared_queue.front();
        shared_queue.pop();
        shared_queue_size--;
        UNLOCK(shared_queue_lock);
        // Critical Section End
        for (auto it : graph[act.user_id])
        {
            act.priority = neighbours[make_pair(act.user_id, it)];
            //  Critical Section Start
            LOCK(feed_queue_lock[it]);
            User[it].order ? User[it].fq2.push(act) : User[it].fq1.push(act);
            feed_queue_size[it]++;
            sprintf(buff, "Pushing %d from %d feed queue.\n", it, act.user_id);
            PRINT(buff);
            UNLOCK(feed_queue_lock[it]);
            // Critical Section End
            //  Critical Section Start
            LOCK(cfq_lock);
            cfq.push(it);
            cfq_size++;
            pthread_cond_broadcast(&cfq_cond);

            UNLOCK(cfq_lock);

            // Critical Section End
        }
    }
    pthread_exit(NULL);
}

void *readPost(void *) // feed update to user
{
    memset(buff, 0, 100);
    while (1)
    {
        //  Critical Section Start
        LOCK(cfq_lock);
        while (cfq_size == 0)
            pthread_cond_wait(&cfq_cond, &cfq_lock);
        int fr = cfq.front();
        cfq.pop();
        cfq_size--;

        UNLOCK(cfq_lock);

        // Critical Section End

        //  Critical Section Start
        LOCK(feed_queue_lock[fr]);
        if (User[fr].order)
        {
            while (!User[fr].fq2.empty()) //
            {
                action act = User[fr].fq2.top();
                User[fr].fq2.pop();
                feed_queue_size[fr]--;
                sprintf(buff, "I read action number %d of type %s posted by user %d at time %s", act.action_id, (char *)actions_types[act.action_type].c_str(), fr, ctime(&act.time_stamp));
                PRINT(buff);
            }
        }
        else
        {
            while (!User[fr].fq1.empty()) //
            {
                action act = User[fr].fq1.top();
                User[fr].fq1.pop();
                feed_queue_size[fr]--;
                sprintf(buff, "I read action number %d of type %s posted by user %d at time %s", act.action_id, (char *)actions_types[act.action_type].c_str(), fr, ctime(&act.time_stamp));
                PRINT(buff);
            }
        }
        UNLOCK(feed_queue_lock[fr]);
        // Critical Section End
    }
    pthread_exit(NULL);
}
signed main()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout << COLOR_BLUE << "----PUSH UPDATE SYSTEM FOR SOCIAL MEDIA SYSTEM ----" << COLOR_RESET << endl;
    // cond inits
    pthread_cond_init(&shared_queue_cond, NULL);
    pthread_cond_init(&cfq_cond, NULL);

    // mutex inits
    pthread_mutex_init(&shared_queue_lock, NULL);
    pthread_mutex_init(&cfq_lock, NULL);
    for (int i = 0; i < 100; i++)
        pthread_mutex_init(&feed_queue_lock[i], NULL);
    srand(time(NULL));
    setvbuf(outputfile, NULL, _IONBF, 0);

    actions_types[0] = "post";
    actions_types[1] = "comment";
    actions_types[2] = "like";
    char buff[100];
    sprintf(buff, "Main_Thread ::Main thread awoke\n");
    PRINT(buff); //
    ifstream inp(filename);
    string line;
    getline(inp, line);
    cout << " -> Reading the CSV File for Input :::" << endl;
    while (getline(inp, line))
    {
        int u, v;
        sscanf(line.c_str(), "%d,%d", &u, &v);
        if (graph.find(u) != graph.end())
            User[u].id = u;
        if (graph.find(v) != graph.end())
            User[v].id = v;
        degree[u]++;
        degree[v]++;
        graph[u].insert(v);
        graph[v].insert(u);
    }
#ifdef DEBUG_LOAD
    std::ofstream file("degree.txt");
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
    std::ofstream file1("output_graph.txt");
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
    cout << " -> Precomputing the Priority of the Nodes's Neighbours for Every Node :::" << endl;
    // priority_map.clear();
    // precomutePriority();
    cout << " * Precomputation Done" << endl;
#ifdef DEBUG_PRIORITY
    ofstream file("priority.txt");

    if (!file)
    {
        cerr << "Unable to open "
             << "priority.txt" << endl;
        exit(EXIT_FAILURE);
    }
    for (auto it : priority_map)
    {
        for (auto x : it.second)
        {
            cout << it.first << " " << x.first << " " << x.second << endl;
        }
    }
    if (file.is_open())
        file.close();
#endif
    pthread_t userSimulator_thread, pushUpdate_thread[25], readPost_thread[10];
    int ret = 0;
    pthread_create(&userSimulator_thread, NULL, userSimulator, NULL);

    for (int i = 0; i < PUSH_UPDATE; i++)
    {
        ret = pthread_create(&pushUpdate_thread[i], NULL, pushUpdate, NULL);
        if (ret != 0)
        {
            printf("Error in creating thread %d\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < READ_POST; i++)

    {
        ret = pthread_create(&readPost_thread[i], NULL, readPost, NULL);
        if (ret != 0)
        {
            printf("Error in creating thread %d\n", i);
            exit(1);
        }
    }

    pthread_join(userSimulator_thread, NULL);

    for (int i = 0; i < 25; i++)
    {
        ret = pthread_join(pushUpdate_thread[i], NULL);
        if (ret != 0)
        {
            printf("Error in creating thread %d\n", i);
            exit(1);
        }
    }
    for (int i = 0; i < 10; i++)

    {
        ret = pthread_join(readPost_thread[i], NULL);
        if (ret != 0)
        {
            printf("Error in creating thread %d\n", i);
            exit(1);
        }
    }
    fclose(outputfile);
    // pthread destroy
    int i = 0;
    while (i < 100)
    {
        pthread_mutex_destroy(&feed_queue_lock[i]);
        i++;
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

    pthread_cond_destroy(&shared_queue_cond);
    pthread_cond_destroy(&cfq_cond);
    pthread_mutex_destroy(&shared_queue_lock);
    pthread_mutex_destroy(&cfq_lock);
    pthread_exit(NULL);
}

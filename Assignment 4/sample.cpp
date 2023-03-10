#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

// g++ -pthread -std=c++11 -o a.out main.cpp
#define mod 1000000007
#define total_users 37700
#define total_edges 289003
#define ordered_set tree<int, null_type, less<int>, rb_tree_tag, tree_order_statistics_node_update> //

using namespace __gnu_pbds;
using namespace std;


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
queue <int> cfq;
pthread_mutex_t cfq_lock;
pthread_cond_t cfq_cond;
int cfq_size = 0;

pthread_mutex_t feed_queue_lock[total_users];
int feed_queue_size[total_users];
// pthread_cond_t feed_queue_cond;

FILE *outputfile = fopen("sns.log", "wb");

void printgraph()
{
    ofstream out("graph_out.txt");
    for (int i = 0; i < total_users; i++)
    {
        out << "User " << i << " : ";
        for (auto it : graph[i])
        {
            out << it << " ";
        }
        out << endl;
    }
    out.close();
}

void printFunc( char *buff )
{
     int buff_size = sizeof(char) * strlen(buff);
     fwrite(buff, sizeof(char), buff_size, outputfile);
     fwrite(buff, sizeof(char), buff_size, stdout);
}
void calculate_common_neigbours(int u, int v)
{
    int cnt = 0;
    for (auto it : graph[u])
    {
        if (graph[v].find(it) != graph[v].end())
        {
            cnt++;
        }
    }
    neighbours[make_pair(u, v)] = cnt;
    neighbours[make_pair(v, u)] = cnt;
}

void *userSimulator(void *)
{
    char buff[100] ;
    while (1)
    {
    set<int> nodes; // set of nodes to be selected
    while (nodes.size() < 100)
    {
        int random_node = rand() % total_users;
        sprintf(buff, "User_Simulator_Thread :: Selected Node: %d \n", random_node);
        printFunc(buff);
        nodes.insert(random_node);
    }
    for (auto it : nodes)
    {
        printFunc(buff);
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

            pthread_mutex_lock(&shared_queue_lock);

            shared_queue.push(act);
            shared_queue_size++ ; 
            pthread_cond_signal(&shared_queue_cond);

            pthread_mutex_unlock(&shared_queue_lock);

            // Critical Section End

            sprintf(buff, "User_Simulator_Thread :: User %d take action : %s %d on %s", it ,  actions_types[r].c_str(), act.action_id, ctime(&act.time_stamp));
            printFunc(buff);
        }
        for (auto itr : graph[it])
        {
            calculate_common_neigbours(it, itr);
        };
    }
    sleep(120);//
    }
    pthread_exit(NULL);
}

void *pushUpdate(void *) // push update to feed queue
{
    char buff[100];
    while (1)
    {
        // Critical Section Start
        pthread_mutex_lock(&shared_queue_lock);

        while ( shared_queue_size == 0 )
        {
            pthread_cond_wait(&shared_queue_cond, &shared_queue_lock);
        }
        action act = shared_queue.front();
        shared_queue.pop();
        shared_queue_size-- ;

        pthread_mutex_unlock(&shared_queue_lock);

        // Critical Section End


        for (auto it : graph[act.user_id])
        {
            act.priority = neighbours[make_pair(act.user_id, it)];

            //  Critical Section Start

            pthread_mutex_lock(&feed_queue_lock[it]);
            
            if (User[it].order)
                User[it].fq2.push(act);
            else
                User[it].fq1.push(act);
            feed_queue_size[it]++; 
            sprintf(buff, "Push_Update_Thread :: Update from user : %d to feed queue of user : %d \n", act.user_id, it);
            printFunc(buff);

            pthread_mutex_unlock(&feed_queue_lock[it]);

            // Critical Section End
            


            //  Critical Section Start

            pthread_mutex_lock(&cfq_lock);
            
            cfq.push(it);
            cfq_size++ ;
            pthread_cond_signal(&cfq_cond);

            pthread_mutex_unlock(&cfq_lock);

            // Critical Section End
        }
    }
    pthread_exit(NULL);
}

void *readPost(void *) // feed update to user
{
    char buff[100];
    while (1)
    {
        //  Critical Section Start
        pthread_mutex_lock(&cfq_lock);

        while (cfq_size == 0)
        {
            pthread_cond_wait(&cfq_cond, &cfq_lock);
        }
        int k = rand() % ((int)cfq_size);
        int fr = cfq.front();
        cfq.pop();
        cfq_size-- ;

        pthread_mutex_unlock(&cfq_lock);
        
        // Critical Section End

        //  Critical Section Start
        pthread_mutex_lock(&feed_queue_lock[fr]);
        if (User[fr].order)
        {
            while (!User[fr].fq2.empty()) //
            {
                action act = User[fr].fq2.top();
                User[fr].fq2.pop();
                feed_queue_size[fr]-- ;
                sprintf( buff , "Read_Post_Thread ::I read action number %d of type %s posted by user %d at time %s", act.action_id, (char *)actions_types[act.action_type].c_str(), fr, ctime(&act.time_stamp) );
                printFunc(buff);
            }
        }
        else
        {
            while (!User[fr].fq1.empty()) //
            {
                action act = User[fr].fq1.top();
                User[fr].fq1.pop();
                feed_queue_size[fr]-- ;
                sprintf( buff , "Read_Post_Thread ::I read action number %d of type %s posted by user %d at time %s", act.action_id, (char *)actions_types[act.action_type].c_str() , fr, ctime(&act.time_stamp) );
                printFunc(buff);
            }
        }
        pthread_mutex_unlock(&feed_queue_lock[fr]);
        // Critical Section End
    }
    pthread_exit(NULL);
}
signed main()
{
    // cond inits
    pthread_cond_init(&shared_queue_cond, NULL);
    pthread_cond_init(&cfq_cond, NULL);

    // mutex inits
    pthread_mutex_init(&shared_queue_lock, NULL);
    pthread_mutex_init(&cfq_lock, NULL);
    for (int i = 0; i < 100; i++)
    {
        pthread_mutex_init(&feed_queue_lock[i], NULL);
    }

    srand(time(NULL));
    setvbuf(outputfile , NULL, _IONBF, 0);
    ios::sync_with_stdio(false);
    cin.tie(0);
    actions_types[0] = "post";
    actions_types[1] = "comment";
    actions_types[2] = "like";
    char buff[100] ;
    sprintf(buff , "Main_Thread ::Main thread awoke\n" ) ;
    printFunc(buff) ; //
    ifstream inp("musae_git_edges.csv");
    string line;

    getline(inp, line);

    while (getline(inp, line))
    {
        int u, v;
        int ind = line.find(",");
        u = (int)stoi(line.substr(0, ind));
        v = (int)stoi(line.substr(ind + 1));
        if (graph.find(u) != graph.end())
        {
            User[u].id = u;
        }
        if (graph.find(v) != graph.end())
        {
            User[v].id = v;
        }
        degree[u]++;
        degree[v]++;
        graph[u].insert(v);
        graph[v].insert(u);
    }

    pthread_t userSimulator_thread, pushUpdate_thread[25], readPost_thread[10];
    pthread_create(&userSimulator_thread, NULL, userSimulator, NULL);
    for (int i = 0; i < 25; i++)
    {
        pthread_create(&pushUpdate_thread[i], NULL, pushUpdate, NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&readPost_thread[i], NULL, readPost, NULL);
    }
    pthread_join(userSimulator_thread, NULL);
    for (int i = 0; i < 25; i++)
    {
        pthread_join(pushUpdate_thread[i], NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(readPost_thread[i], NULL);
    }
    fclose(outputfile) ;

    // pthread destroy
    pthread_cond_destroy(&shared_queue_cond);
    pthread_cond_destroy(&cfq_cond);
    pthread_mutex_destroy(&shared_queue_lock);
    pthread_mutex_destroy(&cfq_lock);
    for (int i = 0; i < 100; i++)
        pthread_mutex_destroy(&feed_queue_lock[i]);
    return 0;
}

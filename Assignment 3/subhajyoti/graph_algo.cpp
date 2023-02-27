#include <bits/stdc++.h>
using namespace std;
#define ll long long

vector<vector<int> > g;

void print_dist_vec(vector<vector<int> > &dist){
    for(int i=0; i<dist.size(); i++){
        cout<<"dist["<<i<<"]:";
        for(auto x: dist[i])
            cout << x << " ";
        cout << endl;
    }
}

// djikstra algorithm to find shortest path from source to all other nodes in a graph with non-negative edge weights using adjacency list representation of graph
vector<int> djikstra(int src, int n){
    vector<int> dist(n+1, INT_MAX);
    dist[src] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > pq;
    pq.push({0, src});

    while(!pq.empty()){
        int u = pq.top().second;
        pq.pop();

        for(int v: g[u]){
            if(dist[v] > dist[u] + 1){
                dist[v] = dist[u] + 1;
                pq.push({dist[v], v});
            }
        }
    }
    for(int i=0; i<=n; i++)
        if(dist[i] == INT_MAX)
            dist[i] = -1;
    return dist;
}

void update_distance(vector<int> &dist,int u,int v){
    queue<int> q, temp;
    q.push(v);
    dist[v] = dist[u] + 1;
    int val=2;
    while(!q.empty()){
        int x = q.front();
        q.pop();
        for(int y: g[x]){
            if(dist[y] == -1){
                dist[y] = dist[u] + val;
                q.push(y);
            }
            else if(dist[y] > dist[u] + val){
                dist[y] = dist[u] + val;
                temp.push(y);
            }
        }
        if(q.empty()){
            q = temp;
            temp = queue<int>();
            val++;
        }
    }
}

void get_initial_graph(int &n, int &m){
    int la;
    fstream file;
    string word, t, q, filename;

    filename = "file.txt";
    file.open(filename.c_str());
    int lno=0;

    file >> word;
    m = stoi(word);

    file >> word;
    n = stoi(word);

    file >> word;
    la = stoi(word);

    g.clear();
    g.resize(n+1);

    int e=0, u, v;
    while(e<=m){
        file >> word;
        u = stoi(word);

        file >> word;
        v = stoi(word);

        g[u].push_back(v);
        g[v].push_back(u);
        e++;
    }
    file.close();
}

void add_more_edges(vector<pair<int, int> > &new_edges){
    fstream file;
    string word, t, q, filename;
    int u, v;

    new_edges.clear();
    filename = "new_nodes.txt";
    file.open(filename.c_str());
    while(file >> word){
        // cout<<word<<endl;
        u = stoi(word);
        file >> word;
        v = stoi(word);

        g[u].push_back(v);
        g[v].push_back(u);
        new_edges.push_back({u, v});
    }
    file.close();
}

void update_dist_vec(vector<vector<int> > &dist, vector<pair<int, int> > &new_edges){
    for(int i=0; i<dist.size(); i++){
        for(auto x: new_edges){
            if(dist[i][x.first] >=0 || dist[i][x.second] >= 0){
                int a,b;
                if(dist[i][x.first]<0){
                    a=x.second;
                    b=x.first;
                }
                else if(dist[i][x.second]<0){
                    a=x.first;
                    b=x.second;
                }
                else if(dist[i][x.first]>=0 && ((dist[i][x.first]+1) < dist[i][x.second])){
                    a=x.first;
                    b=x.second;
                }
                else if(dist[i][x.second]>=0 && ((dist[i][x.second]+1) < dist[i][x.first])){
                    a=x.second;
                    b=x.first;
                }
                else
                    continue;

                cout<<"i:"<<i<<" a:"<<a<<" b:"<<b<<endl;
                update_distance(dist[i], a, b);
            }
        }
    }
}

int main(){
    int n, m, la;
    get_initial_graph(n,m);

    int N = n/10;
    vector<vector<int> > dist(N+1);
    dist[0] = djikstra(0, n);
    dist[1] = djikstra(6, n);

    print_dist_vec(dist);

    vector<pair<int, int> > new_edges;
    add_more_edges(new_edges);

    cout<< "new_edges: "<<new_edges.size()<<endl;
    update_dist_vec(dist, new_edges);

    print_dist_vec(dist);
    
    return 0;
}
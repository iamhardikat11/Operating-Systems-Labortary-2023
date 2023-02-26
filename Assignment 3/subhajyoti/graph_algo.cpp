#include <bits/stdc++.h>
using namespace std;
#define ll long long

vector<vector<int> > g;

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

// distance of a node from source in unweighted graph from adjacency list representation of graph
vector<int> bfs(int src, int n){
    vector<int> dist(n+1, -1);
    dist[src] = 0;

    queue<int> q;
    q.push(src);

    while(!q.empty()){
        int u = q.front();
        q.pop();

        for(int v: g[u]){
            if(dist[v] == -1){
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return dist;
}

int main(){
    int n, m, la;
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

    vector<int> dist[2];
    dist[0] = djikstra(0, n);
    dist[1] = djikstra(6, n);

    cout<<"dist[0]:";
    for(auto x: dist[0])
        cout << x << " ";
    cout << endl<<"dist[1]:";
    for(auto x: dist[1])
        cout << x << " ";
    cout << endl;

    filename = "new_nodes.txt";
    file.open(filename.c_str());
    vector<pair<int, int> > new_edges;
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

    cout<< "new_edges: "<<new_edges.size()<<endl;
    vector<pair<int, int> > temp;
    for(int i=0; i<2; i++){
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



    cout<<"dist[0]:";
    for(auto x: dist[0])
        cout << x << " ";
    cout << endl<<"dist[1]:";
    for(auto x: dist[1])
        cout << x << " ";
    cout << endl;

    return 0;
}
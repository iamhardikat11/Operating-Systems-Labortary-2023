#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
using namespace std;

// Represent a vertex in the graph
class Vertex {
public:
    int id;
    vector<int> neighbors;
    Vertex(int id) 
    {
        this->id = id;
    }
};

// Represent a graph
class Graph {
public:
    vector<Vertex> vertices;
    unordered_map<int, int> idToIndex;

    Graph(int numVertices) {
        vertices.resize(numVertices);
        for (int i = 0; i < numVertices; i++) {
            vertices[i] = Vertex(i);
            idToIndex[i] = i;
        }
    }

    void addEdge(int source, int dest) {
        vertices[idToIndex[source]].neighbors.push_back(dest);
        // vertices[idToIndex[source]].neighbors.push_back(dest);
    }
};

int main() {
    Graph g = new Graph(4039);
    ifstream file("facebook_combined.txt");
    string line;
    while (getline(file, line)) {
        int source, dest;
        sscanf(line.c_str(), "%d %d", &source, &dest);
        // g.addEdge(source, dest);
    }

    // // Printing the graph
    // for (int i = 0; i < g.vertices.size(); i++) {
    //     cout << "Vertex " << i << " is connected to: ";
    //     for (int j = 0; j < g.vertices[i].neighbors.size(); j++) {
    //         cout << g.vertices[i].neighbors[j] << " ";
    //     }
    //     cout << endl;
    // }

    return 0;
}

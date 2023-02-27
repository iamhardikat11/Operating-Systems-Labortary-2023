#include <bits/stdc++.h>
using namespace std;
int main()
{

}

for (int i = 1; i <= 10; i++)
                {
                    if (fork() == 0)
                    {
                        string s = "graph" + to_string(i) + ".txt";
                        for (int j = 0; j < n / 10; i++)
                        {
                            vector<vector<int>> dist(1);
                            dist[0] = djikstra(j + (i - 1) * (n / 10), n);
                            print_dist_vec(dist, j + (i - 1) * (n / 10), s);
                        }
                    }
                    while (wait(NULL) > 0)
                    {
                        ;
                    }
                }
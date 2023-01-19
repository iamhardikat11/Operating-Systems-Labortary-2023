#include <bits/stdc++.h>
using namespace std;
struct comp
{
    template <typename T>

    // Comparator function
    bool operator()(const T &s1, const T &s2) const
    {
        if (s1.second == s2.second)
        {
            if (s1.first.compare(s2.first) > 0)
                return 1;
            else
                return 0;
        }
        else
        {
            return s1.second > s2.second;
        }
    };
};
void sort(map<string, int> &M)
{

    set<pair<string, int>, comp> S(M.begin(), M.end());

    for (auto &it : S)
    {
        cout << it.first << ' ' << it.second << endl;
    }
}
int main()
{
    freopen("input.txt", "w", stdout);
    int n = 100;
    vector<string> sub = {"CS", "ECE", "EE", "AGRI", "MECH"};
    map<string, int> name1;
    map<string, int> sub1;
    for (int i = 0; i < n; i++)
    {
        string name = "";
        for (int i = 0; i < 30; i++)
            name.push_back('a' + rand() % 26);
        name1[name]++;
        string su = sub[rand() % 5];
        sub1[su]++;
        int k = rand()%50;
        for(int i=0;i<k;i++)
            cout << name << " " << su << endl;
    }
    sort(sub1);
    return 0;
}
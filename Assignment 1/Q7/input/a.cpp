#include <bits/stdc++.h>
using namespace std;
int main()
{
    int k = 1000;
    freopen("input.txt", "w", stdout);
    for(int i=0;i<k;i++)
    {
        for(int i=0;i<26;i++)
        {
            cout << (char)(('a'+rand()%26)-(rand()%2)*32);
        }
        cout << " ";
        for(int i=0;i<52;i++)
        {
            cout << (char)(('a'+rand()%26)-(rand()%2)*32);
        }
        cout << " ";
        for(int i=0;i<72;i++)
        {
            cout << (char)(('a'+rand()%26)-(rand()%2)*32);
        }
        cout << endl;
    }
}
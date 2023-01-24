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
            if(i==0)
            {
                char ch = (char)(('a'+rand()%26)-(rand()%2)*32);
                if(ch!='p' && ch!='P')
                {
                    cout << ch;
                }
            }
            else{
                char ch = (char)(('a'+rand()%26)-(rand()%2)*32);
                if(ch!='p' && ch!='P')
                {
                    cout << ch;
                }
            }
        }
        cout << " ";
        for(int i=0;i<52;i++)
        {
            char ch = (char)(('a'+rand()%26)-(rand()%2)*32);
            if(ch!='p' && ch!='P')
            {
                    cout << ch;
            }
        }
        cout << " ";
        for(int i=0;i<72;i++)
        {
            char ch = (char)(('a'+rand()%26)-(rand()%2)*32);
            if(ch!='p' && ch!='P')
            {
                    cout << ch;
            }
        }
        cout << endl;
    }
}
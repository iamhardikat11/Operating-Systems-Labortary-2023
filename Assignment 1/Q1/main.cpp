#include <bits/stdc++.h>
using namespace std;

#define int long long
// Function to find the LCM of two numbers
int gcd(int a, int b)
{
    if(b==0) return a;
    else return gcd(b,a%b);
}
int lcm(int a, int b) {
    return (a * b) / (int)(gcd(a, b));
}

signed main() {
    ifstream file("lcm.txt");
    freopen("output.txt","w", stdout);
    int lcm_result = 1;
    int number;
    while (file >> number) {
        int reverse_number = 0;
        while (number > 0) {
            reverse_number = reverse_number * 10 + (number % 10);
            number /= 10;
        }
        cout << reverse_number << ", "<< endl;
        lcm_result = lcm(lcm_result, reverse_number);
    }
    file.close();
    cout << "LCM: " << lcm_result << endl;
    return 0;
}
#include <iostream>
#include <vector>

using namespace std;

int main() {
    int N = 10; // Replace with desired value of N
    int X = 3; // Replace with desired value of X

    vector<vector<int>> sets(X);

    for (int i = 0; i < N; i++)
        sets[i % X].push_back(i);
    for (int i = 0; i < X; i++) {
        cout << "Set " << i + 1 << ": ";
        for (int j = 0; j < sets[i].size(); j++) {
            cout << sets[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}

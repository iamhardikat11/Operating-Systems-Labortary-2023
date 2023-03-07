// #include <iostream>
// #include <thread>
// #include <mutex>
// #include <chrono>
// #include <vector>
// #include <string>
// using namespace std;

// mutex mtx;
// int cnt = 0;
// void print_block() {
//     int n = 10;
//     char h
//     // critical section (exclusive access to std::cout signaled by locking mtx):
//     mtx.lock();
//     for (int i=0; i<n; ++i) { cout << c; }
//     cout << ' ' << cnt++ << endl;
//     cout << 'n' <<endl;
//     mtx.unlock();
// }

// int main ()
// {
//     vector<pthread_t> threads;
//     // vector<thread> threads;
//     // for (int i = 0; i < 5; i++)
//     // {
//     //     ret = pthread_create(&push_updates[i], NULL, pushUpdates, (void *)i);
//     //     if (ret != 0)
//     //     {
//     //         printf("Error: pthread_create() failed\n");
//     //         exit(EXIT_FAILURE);
//     //     }
//     // }
//     for (int i=0; i<5; ++i) {
//         threads.push_back(thread(print_block,10,'*'));
//     }
//     for (auto& th : threads) th.join();
//     return 0;
// }
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>
using namespace std;

mutex mtx;
int cnt = 0;
int x = -1;
void print_block(int n, char c, int j) {
    // critical section (exclusive access to std::cout signaled by locking mtx):
    mtx.lock();
    for (int i=0; i<n; ++i) { cout << c; }
    cout << ' ' << j << endl;
    cout << 'n' <<endl;
    x = j;
    cout << x << endl;
    mtx.unlock();
}

int main ()
{
    vector<thread> threads;
    cout << "Hello" << x << endl;
    for (int i=0; i<5; ++i) {
        threads.push_back(thread(print_block,10,'*', i));
    }
    for (auto& th : threads) th.join();
    return 0;
}
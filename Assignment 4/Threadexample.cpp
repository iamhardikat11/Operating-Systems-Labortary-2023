#include <iostream>
#include <thread>
using namespace std;
void print_number(int num)
{
     cout << "Thread " << num << " started" <<  endl;
    for (int i = 1; i <= 10; i++)
    {
         cout << "Thread " << num << ": " << i <<  endl;
    }
     cout << "Thread " << num << " finished" <<  endl;
}

int main()
{
     thread t1(print_number, 1);
     thread t2(print_number, 2);

     cout << "Main thread started" <<  endl;

    t1.join();
    t2.join();

     cout << "Main thread finished" <<  endl;

    return 0;
}

/*
# To Compile:-
# make
# ./main
# To Clean 
# make clean
*/
#include "guest.hpp"
#include "cleaner.hpp"

using namespace std;

int X, Y, N;
std::vector<Room> hotel;
std::vector<int> guests_priority;
std::vector<vector<int>> cleaner_pre;
std::mutex mtx;
std::condition_variable cv;
std::vector<sem_t> cleaning_semaphores;
std::vector<bool> cleaning_in_progress;

int main()
{
    std::srand(std::time(0));
    do
    {
        std::cout << "Enter values for X, Y, and N (Y > N > X > 1): ";
        std::cin >> X >> Y >> N;
    } while (!(Y > N && N > X && X > 1));
    hotel.resize(N);
    for (int i = 0; i < N; ++i)
    {
        hotel[i].priority = 0;
        hotel[i].occupants = 0;
        hotel[i].prev_guestid = -1;
        hotel[i].last_cleaned = std::chrono::steady_clock::now();
    }
    guests_priority.resize(Y);
    for (int i = 0; i < Y; ++i)
    {
        guests_priority[i] = rand() % Y + 1;
    }
    cleaning_semaphores.resize(Y);
    for (int i = 0; i < Y; i++)
    {
        sem_init(&cleaning_semaphores[i], 0, 0);
    }
    cleaning_in_progress.resize(Y, false);
    cleaner_pre.resize(X);
    for (int i = 0; i < N; i++)
        cleaner_pre[i % X].push_back(i);
    std::vector<std::thread> guest_threads;
    for (int i = 0; i < Y; i++)
    {
        guest_threads.push_back(std::thread(guest_thread, i));
    }
    std::vector<std::thread> cleaner_threads;
    for (int i = 0; i < X; i++)
    {
        cleaner_threads.push_back(std::thread(cleaning_thread, i));
    }
    for (auto &t : guest_threads)
    {
        t.join();
    }
    for (auto &t : cleaner_threads)
    {
        t.join();
    }
    for (int i = 0; i < Y; i++)
    {
        sem_destroy(&cleaning_semaphores[i]);
    }
    return 0;
}

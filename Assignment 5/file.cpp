#include <bits/stdc++.h>
#include <cmath>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <semaphore.h>
using namespace std;

struct Room
{
    int priority;
    int occupants;
    std::chrono::steady_clock::time_point last_cleaned;
};

int X, Y, N;
std::vector<Room> hotel;
std::vector<int> guests_priority;
std::vector<vector<int>> cleaner_pre;
std::mutex mtx;
std::condition_variable cv;
std::vector<sem_t> cleaning_semaphores;
std::vector<bool> cleaning_in_progress;
std::vector<std::map<int, int>> mark_clean(Y);

int allocate_room(int guest_id)
{
    int idx = -1;
    int min_priority = Y + 1;

    for (int i = 0; i < N; ++i)
    {
        if (hotel[i].occupants < 2)
        {
            if (hotel[i].priority == 0)
            {
                idx = i;
                break;
            }
            else if (hotel[i].priority < guests_priority[guest_id] && hotel[i].priority < min_priority)
            {
                idx = i;
                min_priority = hotel[i].priority;
            }
        }
    }

    if (idx != -1)
    {
        hotel[idx].priority = guests_priority[guest_id];
        hotel[idx].occupants++;
    }

    return idx;
}

void release_room(int room_idx)
{
    hotel[room_idx].priority = 0;
}

bool is_cleaning_needed()
{
    for (const auto &room : hotel)
    {
        if (room.occupants < 2)
        {
            return false;
        }
    }
    return true;
}

void guest_thread(int guest_id)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []
                {
                for (int i = 0; i < Y; ++i) {
                    if (cleaning_in_progress[i]) {
                        return false;
                    }
                }
            return true; });
        
        int room_idx = allocate_room(guest_id);
        if (room_idx != -1)
        {
            std::cout << "For Guest " << guest_id << " allocating " << room_idx << std::endl;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(3));
            lock.lock();
            release_room(room_idx);
            if (is_cleaning_needed())
            {
                for (int i = 0; i < Y; ++i)
                {
                    sem_post(&cleaning_semaphores[i]);
                }
            }
        }
    }
}

void clean_rooms(int cleaner_id, vector<int> rooms)
{
    for (int i = 0; i < rooms.size(); ++i)
    {
        if (hotel[rooms[i]].occupants >= 2)
        {
            std::cout << "Cleaning " << rooms[i] << std::endl;
            hotel[rooms[i]].occupants = 0;
            hotel[rooms[i]].last_cleaned = std::chrono::steady_clock::now();
        }
    }
}
int min(int a, int b)
{
    return a > b ? b : a;
}
void cleaning_thread(int cleaner_id)
{
    while (true)
    {
        for (int i = 0; i < cleaner_pre[cleaner_id].size(); i++)
        {
            sem_wait(&cleaning_semaphores[cleaner_pre[cleaner_id][i]]);
        }
        std::unique_lock<std::mutex> lock(mtx);

        for (int i = 0; i < cleaner_pre[cleaner_id].size(); ++i)
        {
            if (hotel[cleaner_pre[cleaner_id][i]].occupants >= 2)
                cleaning_in_progress[cleaner_pre[cleaner_id][i]] = true;
        }
        clean_rooms(cleaner_id, cleaner_pre[cleaner_id]);
        for (int i = 0; i < cleaner_pre[cleaner_id].size(); ++i)
        {
            cleaning_in_progress[cleaner_pre[cleaner_id][i]] = false;
        }
        bool all_cleaned = true;
        for (int i = 0; i < cleaner_pre[cleaner_id].size(); ++i)
        {
            if(hotel[cleaner_pre[cleaner_id][i]].occupants == 0)
            {
                continue;
            }
            else
            {
                all_cleaned = false;
                break;
            }
        }
        if (all_cleaned)
        {
            if (!is_cleaning_needed())
            {
                for (int i = 0; i < Y; i++)
                {
                    sem_post(&cleaning_semaphores[i]);
                }
            }
            cv.notify_all();
        }
    }
}

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
        hotel[i].last_cleaned = std::chrono::steady_clock::now();
    }
    guests_priority.resize(Y);
    for (int i = 0; i < Y; ++i)
        guests_priority[i] = rand() % Y + 1;
    cleaning_semaphores.resize(Y);
    for (int i = 0; i < Y; i++)
    {
        sem_init(&cleaning_semaphores[i], 0, 0);
    }
    cleaning_in_progress.resize(Y, false);
    cleaner_pre.resize(X);
    for(int i = 0; i < N; i++)
        cleaner_pre[i % X].push_back(i);
    std::vector<std::thread> guest_threads;
    for (int i = 0; i < Y; i++)
        guest_threads.push_back(std::thread(guest_thread, i));
    std::vector<std::thread> cleaner_threads;
    for (int i = 0; i < X; i++)
        cleaner_threads.push_back(std::thread(cleaning_thread, i));
    for (auto &t : guest_threads)
        t.join();
    for (auto &t : cleaner_threads)
        t.join();
    for (int i = 0; i < Y; i++)
    {
        sem_destroy(&cleaning_semaphores[i]);
    }
    return 0;
}

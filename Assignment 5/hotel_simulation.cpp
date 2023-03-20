#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <semaphore.h>

struct Room
{
    int priority;
    int occupants;
    std::chrono::steady_clock::time_point last_cleaned;
};

int X, Y, N;
std::vector<Room> hotel;
std::vector<int> guests_priority;
std::mutex mtx;
std::condition_variable cv;
sem_t cleaning_semaphore;
bool cleaning_in_progress = false;

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
        if (room.occupants >= 2)
        {
            return true;
        }
    }
    return false;
}

void guest_thread(int guest_id)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(rand() % 11 + 10));

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []
                { return !cleaning_in_progress; });

        int room_idx = allocate_room(guest_id);

        if (room_idx != -1)
        {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(rand() % 21 + 10));
            lock.lock();

            release_room(room_idx);

            if (is_cleaning_needed())
            {
                sem_post(&cleaning_semaphore);
            }
        }
    }
}

void clean_room(int room_idx)
{
    std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - hotel[room_idx].last_cleaned));
    hotel[room_idx].occupants = 0;
    hotel[room_idx].last_cleaned = std::chrono::steady_clock::now();
}

void cleaning_thread(int cleaner_id)
{
    while (true)
    {
        sem_wait(&cleaning_semaphore);

        std::unique_lock<std::mutex> lock(mtx);
        cleaning_in_progress = true;

        for (int i = 0; i < N; ++i)
        {
            if (hotel[i].occupants >= 2)
            {
                clean_room(i);
            }
        }

        cleaning_in_progress = false;
        cv.notify_all();
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
    {
        guests_priority[i] = rand() % Y + 1;
    }

    sem_init(&cleaning_semaphore, 0, 0);

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

    sem_destroy(&cleaning_semaphore);

    return 0;
}

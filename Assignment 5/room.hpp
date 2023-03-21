#ifndef _ROOM_
#define _ROOM_
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <condition_variable>

#include <mutex>
#include <semaphore.h>
#include <thread>

using namespace std;

struct Room
{
    int priority;
    int occupants;
    int prev_guestid;
    std::chrono::steady_clock::time_point last_cleaned;
};


int allocate_room(int guest_id);
bool is_cleaning_needed();
void clean_rooms(int thread_idx, vector<int> rooms);
void release_room(int room_idx);
int min(int a, int b);
#endif
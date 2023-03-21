#include "room.hpp"
#include <thread>

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
        if (hotel[idx].prev_guestid != -1)
        {
            cout << "If guest with lower priority removed: Guest " << hotel[idx].prev_guestid << " with priority " << guests_priority[hotel[idx].prev_guestid] << " is leaving room " << idx << endl;
        }
        hotel[idx].prev_guestid = guest_id;
        hotel[idx].priority = guests_priority[guest_id];
        hotel[idx].occupants++;
        cout << "Guest " << guest_id << " with priority " << guests_priority[guest_id] << " is allocated room " << idx << endl;
    }
    // if (idx != -1)
    //     cout<<"Guest "<<guest_id<<" with priority " << guests_priority[guest_id] << " is allocated room "<<idx<<endl;

    return idx;
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

void clean_rooms(int thread_idx, vector<int> rooms)
{
    for (int i = 0; i < rooms.size(); ++i)
    {
        if (hotel[rooms[i]].occupants >= 2)
        {
            cout << "Room " << rooms[i] << " cleaning started" << endl;
            hotel[rooms[i]].occupants = 0;
            hotel[rooms[i]].last_cleaned = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - hotel[rooms[i]].last_cleaned));
            cout << "Room " << rooms[i] << " cleaning done" << endl;
        }
    }
    
    
}

void release_room(int room_idx)
{
    hotel[room_idx].priority = 0;
}

int min(int a, int b)
{
    return a > b ? b : a;
}
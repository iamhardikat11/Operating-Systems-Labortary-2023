#include "guest.hpp"

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
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(rand() % 21 + 10));
            lock.lock();
            if (hotel[room_idx].prev_guestid == guest_id)
            {
                cout << "Time out: Guest " << guest_id << " with priority " << guests_priority[guest_id] << " is leaving room " << room_idx << endl;
                hotel[room_idx].prev_guestid = -1;
            }
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
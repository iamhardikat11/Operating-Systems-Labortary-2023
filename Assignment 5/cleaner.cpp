#include "cleaner.hpp"

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
        if (!is_cleaning_needed())
        {
            cv.notify_all();
        }
    }
}
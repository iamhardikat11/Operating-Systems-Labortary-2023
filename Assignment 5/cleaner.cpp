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
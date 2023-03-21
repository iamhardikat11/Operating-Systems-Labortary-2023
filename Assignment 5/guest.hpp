#include "room.hpp"

extern int X, Y, N;

extern std::vector<Room> hotel;
extern std::vector<int> guests_priority;
extern std::vector<vector<int> > cleaner_pre;
extern std::mutex mtx;
extern std::condition_variable cv;
extern std::vector<sem_t> cleaning_semaphores;
extern std::vector<bool> cleaning_in_progress;

void guest_thread(int guest_id);
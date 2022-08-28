#include "w_opt_rwlock.h"

void WriteOptimizedRWLock::reader_lock()
{
    std::unique_lock<std::mutex> state_lock(mtx);
    while (waiting_writers > 0 || is_a_writer_active)
    {
        cv.wait(state_lock);
    }
    active_readers++;
}

void WriteOptimizedRWLock::reader_unlock()
{
    std::unique_lock<std::mutex> state_lock(mtx);
    active_readers--;
    if (active_readers == 0)
    {
        cv.notify_all();
    }
}

void WriteOptimizedRWLock::writer_lock()
{
    std::unique_lock<std::mutex> state_lock(mtx);
    waiting_writers++;
    while (active_readers > 0 || is_a_writer_active)
    {
        cv.wait(state_lock);
    }
    waiting_writers--;
    is_a_writer_active = true;
}

void WriteOptimizedRWLock::writer_unlock()
{
    std::unique_lock<std::mutex> state_lock(mtx);
    is_a_writer_active = false;
    cv.notify_all();
}

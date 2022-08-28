#ifndef W_OPT_RWLOCK_H
#define W_OPT_RWLOCK_H

#include <mutex>
#include <condition_variable>

#include "rwlock.h"

// Not using std::shared_mutex and std::mutex for this.
// Implementation based on the method found in the below link.
// https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock#Using_a_condition_variable_and_a_mutex
class WriteOptimizedRWLock : public RWLock
{
public:
    virtual void reader_lock();
    virtual void reader_unlock();
    virtual void writer_lock();
    virtual void writer_unlock();

private:
    std::condition_variable cv;
    std::mutex mtx;
    uint64_t active_readers = 0;
    uint64_t waiting_writers = 0;
    bool is_a_writer_active = false;
};

#endif // W_OPT_RWLOCK_H

#ifndef RWLOCK_H
#define RWLOCK_H

class RWLock
{
public:
    virtual void reader_lock() = 0;
    virtual void reader_unlock() = 0;
    virtual void writer_lock() = 0;
    virtual void writer_unlock() = 0;
};

#endif // RWLOCK_H

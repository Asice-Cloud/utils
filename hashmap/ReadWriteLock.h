// ReadWriteLock.h
#pragma once
#include <shared_mutex>

class ReadWriteLock {
private:
    mutable std::shared_mutex mutex;

public:
    void lockRead() const {
        mutex.lock_shared();
    }

    void unlockRead() const {
        mutex.unlock_shared();
    }

    void lockWrite() {
        mutex.lock();
    }

    void unlockWrite() {
        mutex.unlock();
    }
};
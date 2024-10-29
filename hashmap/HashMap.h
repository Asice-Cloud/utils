#pragma once
#include "RBTree.h"
#include "ReadWriteLock.h"
#include <functional>
#include <list>
#include <vector>

template <typename K, typename V> struct Bucket {
  //enum class Type { LIST, TREE } type;
  //std::list<std::pair<K, V>> *list;
  RBTree<K, V> *tree;

  Bucket()
      : //type(Type::LIST), list(new std::list<std::pair<K, V>>()),
        tree(new RBTree<K, V>()) {}
  ~Bucket() {
    //delete list;
    delete tree;
  }
};

template<typename K, typename V>
class HashMap {
private:
    static const size_t INITIAL_CAPACITY = 16;
    static constexpr double LOAD_FACTOR = 0.75;

    std::vector<Bucket<K, V>> buckets;
    size_t size;
    size_t capacity;
    mutable ReadWriteLock rwLock;

    size_t hash(const K& key) const {
        size_t hashValue = std::hash<K>{}(key);
        hashValue ^= (hashValue >> 20) ^ (hashValue >> 12);
        return (hashValue ^ (hashValue >> 7) ^ (hashValue >> 4)) % capacity;
    }

    void resize();

public:
    HashMap() : buckets(INITIAL_CAPACITY), size(0), capacity(INITIAL_CAPACITY) {}
    void put(const K& key, const V& value);
    bool get(const K& key, V& value) const;
    bool remove(const K& key);
    size_t getSize() const {
        rwLock.lockRead();
        size_t currentSize = size;
        rwLock.unlockRead();
        return currentSize;
    }
    bool isEmpty() const {
        rwLock.lockRead();
        bool empty = size == 0;
        rwLock.unlockRead();
        return empty;
    }
};

template<typename K, typename V>
void HashMap<K, V>::put(const K& key, const V& value) {
    rwLock.lockWrite();
    if (size >= capacity * LOAD_FACTOR) {
        resize();
    }

    size_t h = hash(key);
    Bucket<K, V>& bucket = buckets[h];
    V oldValue;
    if (bucket.tree->find(key, oldValue)) {
        bucket.tree->remove(key);
        size--;
    }
    bucket.tree->insert(key, value);
    size++;
    rwLock.unlockWrite();
}

template<typename K, typename V>
bool HashMap<K, V>::get(const K& key, V& value) const {
    rwLock.lockRead();
    size_t h = hash(key);
    const Bucket<K, V>& bucket = buckets[h];
    bool found = bucket.tree->find(key, value);
    rwLock.unlockRead();
    return found;
}

template<typename K, typename V>
bool HashMap<K, V>::remove(const K& key) {
    rwLock.lockWrite();
    size_t h = hash(key);
    Bucket<K, V>& bucket = buckets[h];
    bool removed = bucket.tree->remove(key);
    if (removed) {
        size--;
    }
    rwLock.unlockWrite();
    return removed;
}

template<typename K, typename V>
void HashMap<K, V>::resize() {
    size_t oldCapacity = capacity;
    capacity *= 2;
    std::vector<Bucket<K, V>> newBuckets(capacity);

    for (auto& oldBucket : buckets) {
        oldBucket.tree->inorderTraversal([this, &newBuckets](const K& key, const V& value) {
            size_t newHash = hash(key);
            newBuckets[newHash].tree->insert(key, value);
        });
    }

    buckets = std::move(newBuckets);
}
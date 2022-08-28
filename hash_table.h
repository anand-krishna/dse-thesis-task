#ifndef HASH_H
#define HASH_H

#include "w_opt_rwlock.h"

#include <string>
#include <optional>

namespace containers
{
    /*
        HashTable =
            [
                ptr ---> LinkedListItem_1 -> LinkedListItem_2 ... -> LinkedListItem_n,
                .
                .
                .
                ptr ---> LinkedListItem_1,
                .
                .
                .
                ptr ---> HashTableBucketInstance_n = LinkedListItem_1 -> LinkedListItem_2
            ]

    */

    // Synchronized internally.
    class HashTable
    {
        struct LinkedListItem
        {
            std::string key;
            std::string value;
            LinkedListItem *next;
            LinkedListItem(std::string key, std::string value) : key(key), value(value)
            {
                next = nullptr;
            }
        };

    public:
        HashTable(size_t size);
        ~HashTable();
        void insert(std::string key, std::string value);
        void delete_entry(std::string key);
        std::optional<std::string> read(std::string key);
        void deallocate();
    private:
        LinkedListItem **buckets;
        size_t table_size;
        std::hash<std::string> hash_function = std::hash<std::string>();
        WriteOptimizedRWLock lock;
    };

}

#endif // HASH_H

#include "hash_table.h"

#include <iostream>

containers::HashTable::HashTable(size_t initial_size)
{
    table_size = initial_size;
    buckets = new LinkedListItem *[initial_size];
    for (size_t idx = 0; idx < table_size; idx++)
        buckets[idx] = nullptr;
}

containers::HashTable::~HashTable()
{
    for (size_t idx = 0; idx < table_size; idx++)
    {
        LinkedListItem *head = buckets[idx];
        LinkedListItem *previous = nullptr;
        while (head != nullptr)
        {
            head = head->next;
            delete previous;
            previous = head;
        }
        buckets[idx] = nullptr;
    }
    delete[] buckets;
}

void containers::HashTable::insert(std::string key, std::string value)
{
    lock.writer_lock();
    int hash_value = hash_function(key);
    uint64_t bucket_index = hash_value % table_size;

    LinkedListItem *list = buckets[bucket_index];

    if (list == nullptr)
    {
        // First insert.
        std::cout << "\t\t[HashTable::insert] New element.\n";
        list = new LinkedListItem(key, value);
        buckets[bucket_index] = list;
    }
    else
    {
        // Subsequent inserts.
        std::cout << "\t\t[HashTable::insert] Collision.\n";
        LinkedListItem *head = list;
        while (head->next != nullptr)
        {
            if (head->key == key)
            {
                lock.writer_unlock();
                throw std::runtime_error("Insert Failed. Entry with key: {} already present in the hash table with value: {}.\n");
            }
            head = head->next;
        }
        if (head->key == key)
        {
            lock.writer_unlock();
            throw std::runtime_error("Insert Failed. Entry with key: {} already present in the hash table with value: {}.\n");
        }
        head->next = new LinkedListItem(key, value);
    }

    lock.writer_unlock();
}

std::optional<std::string> containers::HashTable::read(std::string key)
{
    lock.reader_lock();

    int hash_value = hash_function(key);
    uint64_t bucket_index = hash_value % table_size;
    LinkedListItem *list = buckets[bucket_index];

    std::optional<std::string> ret_val = std::nullopt;

    if (list == nullptr)
    {
        std::cout << "\t\t[HashTable::read] No element.\n";
        // Do nothing.
    }
    else
    {
        std::cout << "\t\t[HashTable::read] Searching for element.\n";
        LinkedListItem *head = list;
        while (head != nullptr)
        {
            if (head->key == key)
            {
                ret_val = head->value;
                break;
            }
            head = head->next;
        }
    }

    lock.reader_unlock();

    return ret_val;
}

void containers::HashTable::delete_entry(std::string key)
{
    lock.writer_lock();
    int hash_value = hash_function(key);
    uint64_t bucket_index = hash_value % table_size;
    LinkedListItem *list = buckets[bucket_index];

    if (list == nullptr)
    {
        std::cout << "\t\t[HashTable::delete_entry] No element.\n";
        lock.writer_unlock();
        throw std::runtime_error("Delete failed. Cannot find entry with key: {} in the hash table.\n");
    }
    else
    {
        std::cout << "\t\t[HashTable::read] Trying to delete element.\n";
        bool is_not_deleted = true;
        LinkedListItem *head = list;
        LinkedListItem *old_entry = nullptr;
        while (head != nullptr)
        {
            if (head->key == key)
            {
                if (old_entry == nullptr)
                {
                    // First entry.
                    old_entry = head;
                    head = head->next;
                    delete old_entry;
                }
                else
                {
                    // Subsequent entries.
                    old_entry->next = head->next;
                    delete head;
                }
                is_not_deleted = false;
                break;
            }
            old_entry = head;
            head = head->next;
        }
        if (is_not_deleted)
        {
            std::cout << "\t\t[HashTable::read] Could not find element to delete.\n";
            lock.writer_unlock();
            throw std::runtime_error("Delete failed. Cannot find entry with key: {} in the hash table.\n");
        }
    }

    lock.writer_unlock();
}

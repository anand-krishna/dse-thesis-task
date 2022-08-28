#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>

#include <string>
#include <ostream>

const char *shared_memory_name = "/shared_memory";

const char *client_read_semaphore_name = "/client_read_semaphore";
const char *client_write_semaphore_name = "/client_write_semaphore";
const char *server_semaphore_name = "/server_read_semaphore";

constexpr uint64_t key_size = 256;
constexpr uint64_t value_size = 256;
constexpr uint64_t message_size = 256;
constexpr uint64_t id_size = 256;

enum OperationType
{
    Read,
    Insert,
    Delete
};

enum OperationStatus
{
    Success,
    Failure
};

// std::string does not work. Too many seg faults.
// https://stackoverflow.com/questions/32581057/shared-memory-of-stdstring-give-segmentation-fault-linux
struct OperationData
{
    char key[key_size];
    char value[value_size];
    char id[id_size];
    OperationType type;
    OperationStatus status;
    char message[message_size];

    friend std::ostream &operator<<(std::ostream &os, const OperationData &data);
};

std::ostream &operator<<(std::ostream &os, const OperationData &data)
{
    std::string string_to_print = "[OperationData]: id: " + std::string(data.id) + " key: " + std::string(data.key) + " value: " + std::string(data.value) + " OperationType: " + std::to_string(data.type) + " message: " + std::string(data.message) + " status: " + std::to_string(data.status);
    os << string_to_print;

    return os;
}

#endif // MEMORY_H

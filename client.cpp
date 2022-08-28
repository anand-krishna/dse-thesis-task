#include <iostream>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "memory.h"

int main(int argc, char *argv[])
{
    OperationStatus operation_status;
    OperationType operation_type;
    std::string key;
    std::string value;
    std::string id;
    std::string message;

    if (argc < 4)
    {
        std::cout << "Usage: client <id> <operation> <key> <value>\n\tFor e.g., client insert 10 10\n\tclient delete 10\n";
        std::exit(1);
    }
    // Map/Table would be better...
    if (strcmp(argv[2], "insert") == 0)
        operation_type = OperationType::Insert;
    else if (strcmp(argv[2], "read") == 0)
        operation_type = OperationType::Read;
    else if (strcmp(argv[2], "delete") == 0)
        operation_type = OperationType::Delete;
    else
    {
        std::cout << "Only valid operations are - read, insert, delete\n";
        std::exit(1);
    }

    id = argv[1];
    key = argv[3];

    if (operation_type == OperationType::Insert)
    {
        if (argc < 5)
        {
            std::cout << "Need a valid value for insert operation\n";
            std::exit(1);
        }
        else
        {
            value = argv[4];
        }
    }

    std::cout << "Starting the Client with key: " << key << " value: " << value << " id: " << id << " operation type: " << std::to_string(operation_type) << "\n";

    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0777);

    if (shm_fd == -1)
    {
        std::cout << "Cannot open SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    ftruncate(shm_fd, operation_data_size);

    if (ftruncate(shm_fd, operation_data_size) == -1)
    {
        std::cout << "Cannot open truncate SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    void *mmap_ret_val = mmap(NULL, operation_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (mmap_ret_val == MAP_FAILED)
    {
        std::cout << "Cannot map to SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    OperationData *mapped_data = static_cast<OperationData *>(mmap_ret_val);

    // See comment in server.cpp to understand things better.
    sem_t *client_read_semaphore = sem_open(client_read_semaphore_name, O_EXCL);

    if (client_read_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Client Read Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    sem_t *client_write_semaphore = sem_open(client_write_semaphore_name, O_EXCL);

    if (client_write_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Client Write Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    sem_t *server_semaphore = sem_open(server_semaphore_name, O_EXCL);

    if (server_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Server Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    std::cout << "Waiting to Write to SHM\n";

    if (sem_wait(client_write_semaphore) == -1)
    {
        std::cout << "\t\tError in sem_wait(client_write): " << strerror(errno) << "\n";
    }

    std::cout << "Writing to SHM\n";

    strcpy(mapped_data->id, id.c_str());
    strcpy(mapped_data->key, key.c_str());
    strcpy(mapped_data->value, value.c_str());
    mapped_data->type = operation_type;

    std::cout << "Wrote to SHM: " << *mapped_data << "\n";

    if (sem_post(server_semaphore) == -1)
    {
        std::cout << "\t\tError in sem_post(server): " << strerror(errno) << "\n";
    }

    std::cout << "Waiting to Read from SHM\n";

    if (sem_wait(client_read_semaphore) == -1)
    {
        std::cout << "\t\tError in sem_wait(client_read): " << strerror(errno) << "\n";
    }

    std::cout << "Reading from SHM\n";

    operation_status = mapped_data->status;
    message = std::string(mapped_data->message);
    std::string value_from_server = std::string(mapped_data->value);

    std::cout << "Read from SHM: " << *mapped_data << "\n";

    if (operation_status == OperationStatus::Success)
    {
        std::cout << "Successfully performed operation\n";
        if (operation_type == OperationType::Read)
        {
            std::cout << "Value for key: " << key << " is: " << value_from_server << "\n";
        }
    }
    else
    {
        std::cout << "Operation Failed: " << message << "\n";
    }

    memset(mapped_data, 0, operation_data_size); // Reset for the next client.

    if (sem_post(client_write_semaphore) == -1)
    {
        std::cout << "\t\tError in sem_post(client_write): " << strerror(errno) << "\n";
    }

    // Clean up.

    if (munmap(mapped_data, operation_data_size) == -1)
    {
        std::cout << "Cannot unmap SHM: " << strerror(errno) << "\n";
    }

    if (close(shm_fd) == -1)
    {
        std::cout << "Cannot close SHM fd: " << strerror(errno) << "\n";
    }

    if (sem_close(client_read_semaphore) == -1)
    {
        std::cout << "Cannot close Client Read Semaphore: " << strerror(errno) << "\n";
    }

    if (sem_close(client_write_semaphore) == -1)
    {
        std::cout << "Cannot close Client Write Semaphore: " << strerror(errno) << "\n";
    }

    if (sem_close(server_semaphore) == -1)
    {
        std::cout << "Cannot close Server Semaphore: " << strerror(errno) << "\n";
    }

    return 0;
}

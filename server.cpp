#include <iostream>

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "hash_table.h"
#include "memory.h"

// Singleton instance would be better...
// To free up resources.
int shm_fd;

sem_t *client_read_semaphore;
sem_t *client_write_semaphore;
sem_t *server_semaphore;
OperationData *mapped_data;

bool is_handling_request = false;

void signal_handler(int signal_number);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: server <initial_size_of_hash_table>\n\tFor e.g., server 10\n";
        std::exit(1);
    }

    int hash_table_size = std::stoi(argv[1]);

    containers::HashTable hash_table(hash_table_size);

    signal(SIGINT, signal_handler);

    shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0777);

    if (shm_fd == -1)
    {
        std::cout << "Cannot open SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    if (ftruncate(shm_fd, sizeof(OperationData)) == -1)
    {
        std::cout << "Cannot open truncate SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    mapped_data = static_cast<OperationData *>(mmap(nullptr, sizeof(OperationData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));

    if (mapped_data == MAP_FAILED)
    {
        std::cout << "Cannot map to SHM: " << strerror(errno) << "\n";
        std::exit(1);
    }

    /*

    Init:
        client_reader_sem -> 0
        client_writer_sem -> 1
        server_sem -> 0

    Sync. Logic:
        client                                  server

            sem_wait(client_writer_sem)             while (true):
            .                                           sem_wait(server_sem)
            .                                           .
            .                                           .
            .                                           .
            sem_post(server_sem)                        sem_post(client_reader_sem)
            sem_wait(client_reader_sem)
            .
            .
            .
            sem_post(client_writer_sem)
    */

    client_read_semaphore = sem_open(client_read_semaphore_name, O_CREAT, 0777, 0); // Post in server. Wait in client.

    if (client_read_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Client Read Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    client_write_semaphore = sem_open(client_write_semaphore_name, O_CREAT, 0777, 1); // Post and Wait in client.

    if (client_write_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Client Write Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    server_semaphore = sem_open(server_semaphore_name, O_CREAT, 0777, 0); // Post in client. Wait in server.

    if (server_semaphore == SEM_FAILED)
    {
        std::cout << "Cannot open Server Semaphore: " << strerror(errno) << "\n";
        std::exit(1);
    }

    while (true)
    {
        std::cout << "Listening to requests from Clients\n";

        if (sem_wait(server_semaphore) == -1)
        {
            std::cout << "\t\tError in sem_wait: " << strerror(errno) << "\n";
        }

        std::string key_from_client;
        OperationType type = mapped_data->type;

        try
        {
            std::cout << "\tGot a request from Client with id: " << mapped_data->id << "\n";

            is_handling_request = true;

            if (type == OperationType::Insert)
            {
                key_from_client = std::string(mapped_data->key);
                std::string value_from_client = std::string(mapped_data->value);
                hash_table.insert(key_from_client, value_from_client);
                mapped_data->status = OperationStatus::Success;
            }
            else if (type == OperationType::Delete)
            {
                key_from_client = std::string(mapped_data->key);
                hash_table.delete_entry(key_from_client);
                mapped_data->status = OperationStatus::Success;
            }
            else if (type == OperationType::Read)
            {
                key_from_client = std::string(mapped_data->key);
                std::optional<std::string> value_opt = hash_table.read(key_from_client);
                if (value_opt.has_value())
                {
                    strcpy(mapped_data->value, value_opt.value().c_str());
                    mapped_data->status = OperationStatus::Success;
                }
                else
                {
                    const char *message = "Cannot find the key in the hash table.\n";
                    strcpy(mapped_data->message, message);
                    mapped_data->status = OperationStatus::Failure;
                }
            }
        }
        catch (const std::exception &e)
        {
            mapped_data->status = OperationStatus::Failure;
            strcpy(mapped_data->message, e.what());
            // mapped_data->message = std::string(e.what());
        }
        if (sem_post(client_read_semaphore) == -1)
        {
            std::cout << "\t\tError in sem_post: " << strerror(errno) << "\n";
        }

        is_handling_request = false;

        std::cout << "\tFinished processing a request from Client with id: " << mapped_data->id << "\n";
    }

    return 0;
}

void signal_handler(int signal_number)
{
    if (signal_number == SIGINT)
    {
        while (is_handling_request)
            sleep(1);
        if (munmap(mapped_data, sizeof(OperationData)) == -1)
        {
            std::cout << "Cannot unmap SHM: " << strerror(errno) << "\n";
        }

        if (close(shm_fd) == -1)
        {
            std::cout << "Cannot close SHM fd: " << strerror(errno) << "\n";
        }

        if (shm_unlink(shared_memory_name) == -1)
        {
            std::cout << "Cannot close SHM fd: " << strerror(errno) << "\n";
        }

        if (sem_close(client_read_semaphore) == -1)
        {
            std::cout << "Cannot close Client Read Semaphore: " << strerror(errno) << "\n";
        }

        if (sem_unlink(client_read_semaphore_name) == -1)
        {
            std::cout << "Cannot unlink Client Read Semaphore: " << strerror(errno) << "\n";
        }

        if (sem_close(client_write_semaphore) == -1)
        {
            std::cout << "Cannot close Client Write Semaphore: " << strerror(errno) << "\n";
        }

        if (sem_unlink(client_write_semaphore_name) == -1)
        {
            std::cout << "Cannot unlink Client Write Semaphore: " << strerror(errno) << "\n";
        }

        if (sem_close(server_semaphore) == -1)
        {
            std::cout << "Cannot close Server Semaphore: " << strerror(errno) << "\n";
        }

        if (sem_unlink(server_semaphore_name) == -1)
        {
            std::cout << "Cannot unlink Server Semaphore: " << strerror(errno) << "\n";
        }
    }
}

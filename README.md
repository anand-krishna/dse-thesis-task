# Programming task

* Server
  - Servers requests from a client using a POSIX SHM. Synchronized interally using POSIX semaphores (see the comments in server.cpp)

```console
$ # ./server <initial_size_of_hash_table>
$ ./server 10
$             [HashTable::HashTable]Creating a hash table with size: 10
              [HashTable::HashTable]Created a hash table with size: 10
  Listening to requests from Clients
```

> Note: There is a lot of stuff printed out to the stdout.

* Client
  - Sends requests to the server via a POSIX SHM and is synced using POSIX semaphores (see the comments in server.cpp)

```console
$ # ./client <id> <operation> <key> <value>
$ ./client 123 insert 10 10
Starting the Client with key: 10 value: 20 id: 123 operation type: 1
Waiting to Write to SHM
Writing to SHM
Wrote to SHM: [OperationData]: id: 123 key: 10 value: 20 OperationType: 1 message:  status: 0
Waiting to Read from SHM
Reading from SHM
Read from SHM: [OperationData]: id: 123 key: 10 value: 20 OperationType: 1 message:  status: 0
Successfully performed operation
$ ./client read 10
Starting the Client with key: 10 value:  id: 123 operation type: 0
Waiting to Write to SHM
Writing to SHM
Wrote to SHM: [OperationData]: id: 123 key: 10 value:  OperationType: 0 message:  status: 0
Waiting to Read from SHM
Reading from SHM
Read from SHM: [OperationData]: id: 123 key: 10 value: 20 OperationType: 0 message:  status: 0
Successfully performed operation
Value for key: 10 is: 20
$ ./client delete 10
Starting the Client with key: 10 value:  id: 123 operation type: 2
Waiting to Write to SHM
Writing to SHM
Wrote to SHM: [OperationData]: id: 123 key: 10 value:  OperationType: 2 message:  status: 0
Waiting to Read from SHM
Reading from SHM
Read from SHM: [OperationData]: id: 123 key: 10 value:  OperationType: 2 message:  status: 0
Successfully performed operation
$ ./client 123 read 10
Starting the Client with key: 10 value:  id: 123 operation type: 0
Waiting to Write to SHM
Writing to SHM
Wrote to SHM: [OperationData]: id: 123 key: 10 value:  OperationType: 0 message:  status: 0
Waiting to Read from SHM
Reading from SHM
Read from SHM: [OperationData]: id: 123 key: 10 value:  OperationType: 0 message: Cannot find the key in the hash table.
 status: 1
Operation Failed: Cannot find the key in the hash table.

```

> Note: There is a lot of stuff printed out to the stdout.

---

* Hash Table
  - Uses chaining to address collisions
  - Internally synced using an RWLock (Preferring writers)

* RWLock
  - Abstract type which can be extended for implementing different RWLocks - reader preferred, writer preferred
  - Currently suppporting a [Writer Preferred RWLock](https://en.wikipedia.org/wiki/Readers%E2%80%93writer_lock#Using_a_condition_variable_and_a_mutex)

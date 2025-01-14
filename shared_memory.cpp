#include "shared_memory.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

// Semaphore names
const char *SEM_MUTEX = "/sem_mutex";
const char *SEM_TABLE_READY_PREFIX = "/sem_table_ready_";
const char *SEM_ORDER_READY = "/sem_order_ready";
const char *SEM_ORDER_COMPLETE = "/sem_order_complete";
const char *SEM_CHAIRS_AVAILABLE = "/sem_chairs_available";

// Function to create shared memory
int createSharedMemory(size_t size) {
    int id = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);
    if (id == -1) {
        perror("Shared memory creation failed");
        exit(1);
    }
    return id;
}

// Function to attach to shared memory
void* attachSharedMemory(int id) {
    void* mem = shmat(id, NULL, 0);
    if (mem == (void*)-1) {
        perror("Shared memory attachment failed");
        exit(1);
    }
    return mem;
}

// Function to remove shared memory
void removeSharedMemory(int id) {
    if (shmctl(id, IPC_RMID, NULL) == -1) {
        perror("Shared memory removal failed");
        exit(1);
    }
}

// Function to decrement (lock) a semaphore
void P(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        perror("P() operation failed");
        exit(1);
    }
}

// Function to increment (unlock) a semaphore
void V(sem_t *sem) {
    if (sem_post(sem) == -1) {
        perror("V() operation failed");
        exit(1);
    }
}

// Function to initialize shared memory and semaphores
void initializeSharedMemoryAndSemaphores() {
    int shm_id = createSharedMemory(sizeof(SharedData));
    SharedData *shared_data = (SharedData *)attachSharedMemory(shm_id);

    // Initialize shared memory
    memset(shared_data, 0, sizeof(SharedData));
    for (int i = 0; i < NUM_TABLES; i++) {
        for (int j = 0; j < CHAIRS_PER_TABLE; j++) {
            shared_data->chairs[i][j] = 0;
        }
        shared_data->table_status[i] = 0;
    }
    shared_data->log_index = 0;

    // Initialize semaphores
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open (mutex)");
        exit(1);
    }

    sem_open(SEM_CHAIRS_AVAILABLE, O_CREAT, 0666, NUM_TABLES * CHAIRS_PER_TABLE);
    sem_open(SEM_ORDER_READY, O_CREAT, 0666, 0);
    sem_open(SEM_ORDER_COMPLETE, O_CREAT, 0666, 0);
    sem_open(SEM_TABLE_READY_PREFIX, O_CREAT, 0666, 3);

    // for (int i = 0; i < NUM_TABLES; i++) {
    //     char sem_name[64];
    //     snprintf(sem_name, sizeof(sem_name), "%s%d", SEM_TABLE_READY_PREFIX, i);
    //     sem_open(sem_name, O_CREAT, 0666, 1);
    // }

    std::cout << "Shared memory and semaphores initialized successfully." << std::endl;
    std::cout << "Shared memory ID: " << shm_id << std::endl; // Output shm_id
}

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>
#include <sys/types.h>

// Shared memory constants
#define NUM_TABLES 3
#define CHAIRS_PER_TABLE 4
#define MAX_LOG_SIZE 100
#define LOG_ENTRY_SIZE 256

// Semaphore names
extern const char *SEM_MUTEX;
extern const char *SEM_TABLE_READY_PREFIX;
extern const char *SEM_ORDER_READY;
extern const char *SEM_ORDER_COMPLETE;
extern const char *SEM_CHAIRS_AVAILABLE;

// Enums for mandatory and optional items
enum MandatoryItems {
    NO_MANDATORY = 0,
    WINE = 1,
    WATER = 2,
    WINE_AND_WATER = 3
};

enum OptionalItems {
    NO_OPTIONAL = 0,
    CHEESE = 1,
    SALAD = 2,
    CHEESE_AND_SALAD = 3
};

// Shared memory structure
struct SharedData {
    int chairs[NUM_TABLES][CHAIRS_PER_TABLE]; // 1 = occupied, 0 = available
    int table_status[NUM_TABLES];            // 1 = fully occupied, 0 = available
    int total_visitors;
    int waiting_time_sum;                    // Total waiting time of all visitors
    int visit_time_sum;       
    int total_time_sum;                // Total time visitors spent
    int order_processing_time_sum;           // Total time spent processing orders
    int wine_served;
    int water_served;
    int cheese_served;
    int salad_served;
    char log[MAX_LOG_SIZE][LOG_ENTRY_SIZE];
    int log_index;

    // Fields for current order processing
    MandatoryItems current_order_mandatory;  // Mandatory items ordered
    OptionalItems current_order_optional;    // Optional items ordered
    pid_t current_order_pid;                 // PID of the visitor placing the order
    int current_order_table;                 // Table number of the visitor
    int current_order_chair;                 // Chair number of the visitor
};

int createSharedMemory(size_t size);
void* attachSharedMemory(int id);
void removeSharedMemory(int id);
void P(sem_t *sem);
void V(sem_t *sem);
void initializeSharedMemoryAndSemaphores();


#endif // SHARED_MEMORY_H


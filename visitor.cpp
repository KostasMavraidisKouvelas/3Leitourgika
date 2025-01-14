#include "shared_memory.h"
#include <iostream>
#include <unistd.h>
#include <sys/shm.h>
#include <cstdlib>
#include <fcntl.h>
#include <semaphore.h>
#include <cstring>
#include <ctime>

// Function to calculate random time between a range
int calculateRandomTime(double minFactor, double maxFactor, int baseTime) {
    int minTime = static_cast<int>(minFactor * baseTime);
    int maxTime = static_cast<int>(maxFactor * baseTime);
    return minTime + (rand() % (maxTime - minTime + 1));
}

// Randomly select mandatory and optional items
MandatoryItems getRandomMandatoryOrder() {
    int choice = rand() % 4; // 0 = NO_MANDATORY, 1 = WINE, 2 = WATER, 3 = WINE_AND_WATER
    return static_cast<MandatoryItems>(choice);
}

OptionalItems getRandomOptionalOrder() {
    int choice = rand() % 4; // 0 = NO_OPTIONAL, 1 = CHEESE, 2 = SALAD, 3 = CHEESE_AND_SALAD
    return static_cast<OptionalItems>(choice);
}

// Helper function for logging orders
std::string mandatoryItemsToString(MandatoryItems order) {
    switch (order) {
        case WINE: return "wine";
        case WATER: return "water";
        case WINE_AND_WATER: return "wine and water";
        default: return "none";
    }
}

std::string optionalItemsToString(OptionalItems order) {
    switch (order) {
        case CHEESE: return "cheese";
        case SALAD: return "salad";
        case CHEESE_AND_SALAD: return "cheese and salad";
        default: return "none";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5 || strcmp(argv[1], "-d") != 0 || strcmp(argv[3], "-s") != 0) {
        std::cerr << "Usage: " << argv[0] << " -d resttime -s shmid\n";
        return 1;
    }

    int resttime = std::atoi(argv[2]);
    int shm_id = std::atoi(argv[4]);

    srand(time(NULL) ^ getpid());

    // Attach to shared memory
    SharedData *shared_data = static_cast<SharedData *>(attachSharedMemory(shm_id));
    if (!shared_data) {
        std::cerr << "[Visitor] Failed to attach to shared memory.\n";
        return 1;
    }

    // Open semaphores
    sem_t *mutex = sem_open(SEM_MUTEX, 0);
    sem_t *chairs_available = sem_open(SEM_CHAIRS_AVAILABLE, 0);
    sem_t *order_ready = sem_open(SEM_ORDER_READY, 0);
    sem_t *order_complete = sem_open(SEM_ORDER_COMPLETE, 0);
    sem_t *table_ready = sem_open(SEM_TABLE_READY_PREFIX, 0);

    if (mutex == SEM_FAILED || chairs_available == SEM_FAILED ||
        order_ready == SEM_FAILED || order_complete == SEM_FAILED) 
    {
        std::cerr << "[Visitor] Failed to open one or more semaphores.\n";
        return 1;
    }

    P(table_ready); // Wait for a free chair
    P(chairs_available); // Lock chairs
    P(mutex);

    int myTable = -1, myChair = -1;
    bool lockTable =  0;
    for (int i = 0; i < NUM_TABLES && myTable == -1; i++) {
        if (shared_data->table_status[i] == 0) {
            for (int j = 0; j < CHAIRS_PER_TABLE && myChair == -1; j++) {
                if (shared_data->chairs[i][j] == 0) {
                    shared_data->chairs[i][j] = 1;
                    myTable = i;
                    myChair = j;

                    bool tableIsFull = true;
                    for (int k = 0; k < CHAIRS_PER_TABLE; k++) {
                        if (shared_data->chairs[i][k] == 0) {
                            tableIsFull = false;
                            break;
                        }
                    }
                    if (tableIsFull)
                    {
                        lockTable = 1;
                        shared_data->table_status[i] = 1;
                    }

                    std::cout << "Visitor " << getpid() << " sat at table " << i
                              << ", chair " << j << ".\n";
                    break;
                }
            }
        }
    }
    if(lockTable ==0)
        V(table_ready);
    
    V(mutex);

    MandatoryItems mandatoryOrder = getRandomMandatoryOrder();
    OptionalItems optionalOrder = getRandomOptionalOrder();

    P(mutex);
    shared_data->current_order_mandatory = mandatoryOrder;
    shared_data->current_order_optional = optionalOrder;
    shared_data->current_order_pid = getpid();
    shared_data->current_order_table = myTable;
    shared_data->current_order_chair = myChair;

    std::cout << "Visitor " << getpid() << " placed order: "
              << mandatoryItemsToString(mandatoryOrder) << " and "
              << optionalItemsToString(optionalOrder) << " at table " << myTable
              << ", chair " << myChair << ".\n";

    V(mutex);

    V(order_ready); // Notify receptionist
    P(order_complete); // Wait for order completion

    int stayTime = calculateRandomTime(0.70, 1.0, resttime);
    sleep(stayTime);

    P(mutex);
    shared_data->visit_time_sum += stayTime;
    std::cout << "Visitor " << getpid() << " stayed for " << stayTime << " seconds.\n";
    V(mutex);

    P(mutex);
    shared_data->chairs[myTable][myChair] = 0;

    bool isEmpty = true;
    for (int c = 0; c < CHAIRS_PER_TABLE; c++) {
        if (shared_data->chairs[myTable][c] == 1) {
            isEmpty = false;
            break;
        }
    }
    if (isEmpty)
    { 
        shared_data->table_status[myTable] = 0;
        V(table_ready);
    }
    V(chairs_available);
    V(mutex);

    return 0;
}

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

// Helper function to process mandatory items
void processMandatoryItems(MandatoryItems mandatoryOrder, SharedData *shared_data) {
    switch (mandatoryOrder) {
        case WINE:
            shared_data->wine_served++;
            std::cout << "Receptionist served wine.\n";
            break;
        case WATER:
            shared_data->water_served++;
            std::cout << "Receptionist served water.\n";
            break;
        case WINE_AND_WATER:
            shared_data->wine_served++;
            shared_data->water_served++;
            std::cout << "Receptionist served wine and water.\n";
            break;
        default:
            std::cout << "Receptionist found no mandatory items to serve.\n";
            break;
    }
}

// Helper function to process optional items
void processOptionalItems(OptionalItems optionalOrder, SharedData *shared_data) {
    switch (optionalOrder) {
        case CHEESE:
            shared_data->cheese_served++;
            std::cout << "Receptionist served cheese.\n";
            break;
        case SALAD:
            shared_data->salad_served++;
            std::cout << "Receptionist served salad.\n";
            break;
        case CHEESE_AND_SALAD:
            shared_data->cheese_served++;
            shared_data->salad_served++;
            std::cout << "Receptionist served cheese and salad.\n";
            break;
        default:
            std::cout << "Receptionist found no optional items to serve.\n";
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5 || strcmp(argv[1], "-d") != 0 || strcmp(argv[3], "-s") != 0) {
        std::cerr << "Usage: " << argv[0] << " -d ordertime -s shm_id\n";
        return 1;
    }

    int ordertime = std::atoi(argv[2]); // Maximum order processing time
    int shm_id = std::atoi(argv[4]);   // Shared memory ID

    srand(time(NULL) ^ getpid());

    // Attach to shared memory
    SharedData *shared_data = static_cast<SharedData *>(attachSharedMemory(shm_id));
    if (!shared_data) {
        std::cerr << "[Receptionist] Failed to attach to shared memory.\n";
        return 1;
    }

    // Open semaphores
    sem_t *mutex = sem_open(SEM_MUTEX, 0);
    sem_t *order_ready = sem_open(SEM_ORDER_READY, 0);
    sem_t *order_complete = sem_open(SEM_ORDER_COMPLETE, 0);

    if (mutex == SEM_FAILED || order_ready == SEM_FAILED || order_complete == SEM_FAILED) {
        std::cerr << "[Receptionist] Failed to open one or more semaphores.\n";
        return 1;
    }

    std::cout << "[Receptionist] Started. Waiting for orders...\n";

    while (true) {
        // Wait for an order to be ready
        P(order_ready);

        // Lock shared memory
        P(mutex);

        // Retrieve the current order details
        MandatoryItems mandatoryOrder = static_cast<MandatoryItems>(shared_data->current_order_mandatory);
        OptionalItems optionalOrder = static_cast<OptionalItems>(shared_data->current_order_optional);
        pid_t visitor_pid = shared_data->current_order_pid;
        int table = shared_data->current_order_table;
        int chair = shared_data->current_order_chair;

        std::cout << "Receptionist processing order from visitor PID " << visitor_pid
                  << " at table " << table << ", chair " << chair << ".\n";

        // Unlock shared memory
        V(mutex);

        // Simulate time for preparing the order
        int processingTime = calculateRandomTime(0.50, 1.0, ordertime);
        sleep(processingTime);

        // Lock shared memory again to update stats
        P(mutex);

        // Process the mandatory and optional items
        processMandatoryItems(mandatoryOrder, shared_data);
        processOptionalItems(optionalOrder, shared_data);

        // Increment total order processing time
        shared_data->order_processing_time_sum += processingTime;

        // Log the order completion
        std::cout << "Receptionist completed order for visitor PID " << visitor_pid
                  << " after " << processingTime << " seconds.\n";

        // Unlock shared memory
        V(mutex);

        // Notify the visitor that the order is complete
        V(order_complete);
    }

    return 0;
}

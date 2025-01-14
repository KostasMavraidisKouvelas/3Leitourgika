#include "shared_memory.h"
#include <iostream>
#include <unistd.h>
#include <sys/shm.h>
#include <cstdlib>
#include <fcntl.h>
#include <semaphore.h>
#include <csignal>

volatile bool running = true; // Global flag for graceful termination

// Signal handler for Ctrl+C
void handleSignal(int signal) {
    if (signal == SIGINT) {
        running = false;
    }
}

int main(int argc, char *argv[]) 
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <shm_id> <refresh_interval>\n";
        return 1;
    }

    // Get the shared memory ID and refresh interval
    int shm_id = std::atoi(argv[1]);
    int refresh_interval = std::atoi(argv[2]);

    // Attach to shared memory
    SharedData *shared_data = static_cast<SharedData*>(attachSharedMemory(shm_id));
    if (!shared_data) {
        std::cerr << "[Monitor] Failed to attach to shared memory.\n";
        return 1;
    }

    // Open the mutex semaphore
    sem_t *mutex = sem_open(SEM_MUTEX, 0);
    if (mutex == SEM_FAILED) {
        std::cerr << "[Monitor] Failed to open SEM_MUTEX.\n";
        return 1;
    }

    // Attach signal handler
    std::signal(SIGINT, handleSignal);

    std::cout << "[Monitor] Started. Press Ctrl+C to terminate.\n";

    while (running) {
        // Lock the shared memory for safe access
        P(mutex);

        // Print the current state of the bar
        std::cout << "\n[Monitor] Current state of the bar:\n";
        for (int i = 0; i < NUM_TABLES; i++) {
            std::cout << "  Table " << i << ": ";
            if (shared_data->table_status[i] == 0) {
                std::cout << "Unlocked (Available) | Seats: ";
            } else {
                std::cout << "Locked (Full or Reserved) | Seats: ";
            }
            for (int j = 0; j < CHAIRS_PER_TABLE; j++) {
                std::cout << (shared_data->chairs[i][j] ? "[Occupied]" : "[Free]") << " ";
            }
            std::cout << std::endl;
        }

        // Print statistics
        std::cout << "[Monitor] Stats so far:"
                  << " Wine=" << shared_data->wine_served
                  << ", Water=" << shared_data->water_served
                  << ", Cheese=" << shared_data->cheese_served
                  << ", Salad=" << shared_data->salad_served << "\n";

        // Print log entries (optional)
        std::cout << "[Monitor] Log entries (" << shared_data->log_index << "):\n";
        for (int i = 0; i < shared_data->log_index; i++) {
            std::cout << "  " << shared_data->log[i] << std::endl;
        }

        // Unlock the shared memory
        V(mutex);

        // Wait before the next update
        sleep(refresh_interval);
    }

    // Graceful termination: Calculate and display median visit time
    P(mutex);
    if (shared_data->total_visitors > 0) {
        double medianTime = static_cast<double>(shared_data->visit_time_sum) / shared_data->total_visitors;
        std::cout << "[Monitor] Terminating. Median visit time: " << medianTime << " seconds.\n";
    } else {
        std::cout << "[Monitor] No visitors have been processed.\n";
    }
    V(mutex);

    std::cout << "[Monitor] Exiting gracefully.\n";
    return 0;
}

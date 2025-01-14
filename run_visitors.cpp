#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

void runVisitor(const std::string &visitorPath, int restTime, int shmId) {
    // Construct the command
    std::string command = visitorPath + " -d " + std::to_string(restTime) + " -s " + std::to_string(shmId);
    // Execute the command
    execlp(visitorPath.c_str(), visitorPath.c_str(), "-d", std::to_string(restTime).c_str(),
           "-s", std::to_string(shmId).c_str(), (char *)NULL);
    // If execlp fails
    perror("execlp failed");
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <visitor_path> <shm_id> <num_visitors> <max_resttime>\n";
        return 1;
    }

    // Parse arguments
    std::string visitorPath = argv[1];
    int shmId = std::atoi(argv[2]);
    int numVisitors = std::atoi(argv[3]);
    int maxRestTime = std::atoi(argv[4]);

    if (numVisitors <= 0 || maxRestTime <= 0) {
        std::cerr << "Error: num_visitors and max_resttime must be positive integers.\n";
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Run visitors
    for (int i = 0; i < numVisitors; ++i) {
        int randomRestTime = 10 + rand() % (maxRestTime - 9); // Random rest time between 10 and maxRestTime
        if (fork() == 0) {
            // Child process: Run visitor
            runVisitor(visitorPath, randomRestTime, shmId);
            exit(0); // Ensure child process exits after executing the visitor
        }
        // Parent process: Continue to next visitor
    }

    // Wait for all child processes to finish
    while (wait(NULL) > 0);

    std::cout << "[runvisitors] All visitors have finished.\n";

    return 0;
}

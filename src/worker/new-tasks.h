#ifndef PARALLEL_COMPUTE_H  // Include guard
#define PARALLEL_COMPUTE_H

#include <mutex>
#include <vector>
#include <thread>
#include <iostream>

class ParallelCompute {
private:
    std::mutex mtx;
    std::vector<std::thread> threads;
    
    // Declare private methods
    void printThreadInfo(int core_id, int start, int end, int chunk_size);
    void processChunk(int core_id, int start, int end);

public:
    // Declare public methods
    void compute(int total_range, int num_threads);
    
    // Constructor & Destructor if needed
    ParallelCompute() = default;  // Default constructor
    ~ParallelCompute() = default; // Default destructor
};

#endif
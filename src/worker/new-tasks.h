#ifndef NEW_TASKS_H
#define NEW_TASKS_H

#include <iostream>
#include <cmath>
#include <mutex>
#include <sstream>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <crypt.h>
#include <string>

#define PRINT_THRESHOLD 1000
#define CHAR_SET_SIZE 26
#define LETTERS 7

// Forward declarations of global functions
std::vector<int> base10_to_baseN(u_long num, int base, int size);
std::string indices_to_string(const std::vector<int> indices);
std::string extract_salt(const std::string& tar_hashed);

extern char char_set[CHAR_SET_SIZE + 1];

class ParallelCompute {
private:
    // Shared all threads
    std::mutex mtx;
    std::vector<std::thread> threads;
    bool found = false;

    // Separate for each thread
    std::vector<bool> endflags;
    std::vector<int> counters;
    std::vector<std::string> curr_hashes;

    // Private member function declarations - Match EXACTLY with the implementations
    void write_thread_file(int thread_id, std::string filename, std::string perm_curr);
    
    void brute_force_letters(int thread_id, const std::string tar_hashed, 
                           const std::string salt, std::string perm_curr, 
                           int chunk, const std::vector<int> start_pass, 
                           int depth, bool lock);
    
    void print_thread_info(int core_id, std::vector<int> perm_start, 
                          std::vector<int> perm_end, int chunk_size);
    
    void process_chunk(int core_id, std::string perm_curr, std::string tar_hashed, 
                      std::string salt, std::vector<int> perm_start, 
                      std::vector<int> perm_end, int chunk_size);

public:
    bool compute(std::string tar_hashed, u_int16_t start_idx, u_int16_t end_idx, 
                u_long chunk_size, int cores);
};

#endif // NEW_TASKS_H
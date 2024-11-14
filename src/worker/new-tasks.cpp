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
#include "new-tasks.h"

#define PRINT_THRESHOLD 1000
#define CHAR_SET_SIZE 26
#define LETTERS 7
char char_set[CHAR_SET_SIZE + 1] = "abcdefghijklmnopqrstuvwxyz";

std::vector<int> base10_to_baseN(u_long num, int base, int size) {
    std::vector<int> result;

    // Convert number to the specified base, inserting each digit at the front
    while (num > 0) {
        result.insert(result.begin(), num % base);  // Insert at the front
        num /= base;
    }

    // Add the padding in front
    while (size > result.size()) result.insert(result.begin(), 0); 

    return result;
}

std::string indices_to_string(const std::vector<int> indices) {
    std::string result;
    for (int index : indices) {
        result += 'a' + index;  // Convert index to corresponding character
    }
    return result;
}

std::string extract_salt(const std::string& tar_hashed) {
    std::string hash_str(tar_hashed);
    std::string salt;
    size_t salt_start = hash_str.find("$", 1);
    if (salt_start != std::string::npos) {
        size_t salt_end = hash_str.find("$", salt_start + 1);
        if (salt_end != std::string::npos) {
            salt = hash_str.substr(salt_start + 1, salt_end - salt_start - 1);
        }
    }
    return salt;
}
void ParallelCompute::write_thread_file(int thread_id, std::string filename ,std::string perm_curr) {
        // Printing the result
        std::lock_guard<std::mutex> lock(mtx); 
        std::ofstream logfile(filename, std::ios::app);  // Open in append mode

        if (!logfile.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        logfile << "Combination " << perm_curr << "  value: " << curr_hashes[thread_id] << std::endl;
        logfile.close();
    }

void ParallelCompute::brute_force_letters(int thread_id, const std::string tar_hashed, const std::string salt, 
                        std::string perm_curr, int chunk, 
                        const std::vector<int> start_pass, int depth = 1, 
                        bool lock = true) {
        // Check shared found flag
        if (found || endflags[thread_id]) return;

        // Base case
        if (depth == LETTERS) {
            struct crypt_data data;
            data.initialized = 0;
            curr_hashes[thread_id] = crypt_r(perm_curr.c_str(), salt.c_str(), &data);
            if (curr_hashes[thread_id] == tar_hashed) {
                std::string filename = "success.txt";
                write_thread_file(thread_id, filename, perm_curr);

                std::cout << "Found the pass !: " << perm_curr << '\n';
                std::cout << "Hashed: " << curr_hashes[thread_id] << std::endl;
                found = true;
                return;
            }
            if (counters[thread_id] % PRINT_THRESHOLD == 0) {
                std::lock_guard<std::mutex> lock(mtx); // to check output uncomment this
                // std::string filename = "logs" + std::to_string(thread_id) + ".txt";
                // write_thread_file(thread_id, filename, perm_curr);
                std::cout << "Counter: " << counters[thread_id] << std::endl;
                std::cout << "Combination: " << perm_curr << std::endl;
                std::cout << "Hashed: " << curr_hashes[thread_id] << std::endl;
            }

            if (counters[thread_id] == chunk) {
                // std::cout << "Break Combination: " << perm_curr << std::endl;
                endflags[thread_id] = true;
                return;
            }
            counters[thread_id]++;
            return;
        } else if (depth < LETTERS) {
            int init_idx = 0;
            if (lock) init_idx = start_pass[depth];
            for (int i = init_idx; i < CHAR_SET_SIZE; i++) {
                char tar_char = char_set[i];
                std::string new_perm = perm_curr + tar_char;
                bool next_lock = lock;
                if (i > init_idx) next_lock = false;
                brute_force_letters(thread_id, tar_hashed, salt, new_perm, chunk, start_pass, 
                                depth + 1, next_lock);
            }
        }
    }
    
    // Private helper function
void ParallelCompute::print_thread_info(int core_id, std::vector<int> perm_start, std::vector<int> perm_end, int chunk_size) {
        // Convert all inputs for printing
        std::string start = indices_to_string(perm_start);
        std::string end = indices_to_string(perm_end);

        std::lock_guard<std::mutex> lock(mtx);  // Lock for thread-safe printing
        std::cout << "\n------Thread-Summary-----\n";
        std::cout << "Core_id: " << core_id << std::endl;
        std::cout << "Start: " << start << std::endl;
        std::cout << "End: " << end << std::endl;
        std::cout << "Chunk Size: " << chunk_size << std::endl;
        std::cout << "-------------------------\n";
    }

    // Private worker function
void ParallelCompute::process_chunk(int core_id, std::string perm_curr, std::string tar_hashed, std::string salt, 
                        std::vector<int> perm_start, std::vector<int> perm_end, int chunk_size) {
        // Print initial thread info
        print_thread_info(core_id, perm_start, perm_end, chunk_size);
        
        // Simulate some work
        // std::lock_guard<std::mutex> lock(mtx);
        std::string full_salt = "$6$" + salt;
        brute_force_letters(core_id, tar_hashed, full_salt, perm_curr, chunk_size, perm_start, 1, true);
    }

bool ParallelCompute::compute(std::string tar_hashed, u_int16_t start_idx, u_int16_t end_idx, 
                    u_long chunk_size, int cores) {
        // Clear any existing threads
        threads.clear();
        counters.clear();
        endflags.clear();
        curr_hashes.clear();
        // Resize vectors to match the number of cores
        counters.resize(cores, 0);    // Initialize counters to 0 for each core
        endflags.resize(cores, false); // Initialize endflags to false for each core
        curr_hashes.resize(cores, ""); // Initialize endflags to false for each core

        // Extract salt
        std::string salt = extract_salt(tar_hashed);

        // Get indices
        std::vector<int> start_base26 = base10_to_baseN(start_idx, CHAR_SET_SIZE, LETTERS);
        std::vector<int> end_base26 = base10_to_baseN(end_idx, CHAR_SET_SIZE, LETTERS);
    
        // Print info    
        int new_chunk_size = ceil((chunk_size * (end_idx - start_idx)) / cores);
        std::cout << "Starting computation with " << cores << " threads...\n";
        std::cout << "Target hashed is" << tar_hashed << "\n";
        std::cout << "Target hashed is" << salt << "\n";
        std::vector<u_long> thread_starts(cores);
        std::vector<u_long> thread_ends(cores);
        for(int i = 0; i < cores; i++) {
            // Get the start and end of each thread
            thread_starts[i] = start_idx * chunk_size + i * new_chunk_size;
            thread_ends[i] = thread_starts[i] + ((i + 1) * new_chunk_size);
            
            // Convert int base 10 to array base 26
            std::vector<int> start_pass = base10_to_baseN(thread_starts[i], CHAR_SET_SIZE, LETTERS);
            std::vector<int> end_pass = base10_to_baseN(thread_ends[i], CHAR_SET_SIZE, LETTERS);

            // Get the start and end of each thread
            std::string perm_start = indices_to_string(start_pass);
            std::string perm_end = indices_to_string(end_pass);
            std::string perm_curr(1, perm_start[0]);    // current permutation eg. "a"

            // Create thread with core_id (i+1), start, and end values
            threads.emplace_back(&ParallelCompute::process_chunk, 
                                    this,  // core_id
                                    i,
                                    perm_curr,
                                    tar_hashed,
                                    salt, 
                                    start_pass,
                                    end_pass,
                                    new_chunk_size);
        }       
        // Wait for all threads to complete
        for(auto& t : threads) {
            t.join();
        }
        
        std::cout << "\nComputation completed!\n";
        return found;
    }
//};

// int main() {
//     ParallelCompute computer;
//     // Process 10 items using 4 threads
//     std::string tar_hashed = "$6$.gK6Njlm2G17CumU$3vB12Jo0PhuwIubHfKI8bGqjFrtghTj6eMM7uUV2F5VypgwQ3KDfGSj3ezF2cWJf5CHWNUQ4xuRrBZEY9N1g01";
//     for (int i = 1; i < 4; i++)
//     {
//         uint16_t start_index = i;
//         uint16_t end_index = i + 1;
//         u_long chunk_size = 10000;
//         bool found = computer.compute(tar_hashed, start_index, end_index, chunk_size, 2);
//         if (found) break;
//     }
//     return 0;
// }
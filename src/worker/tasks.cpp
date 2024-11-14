#include <iostream>
#include <cmath>
#include <mutex>
#include <sstream>
#include <vector>
#include <fstream>
#include <thread>
#include <atomic>
#include <crypt.h>
// Created Libs
#include "sha512.h"

#define PRINT_THRESHOLD 1000
#define CHAR_SET_SIZE 27
#define LETTERS 7
char char_set[CHAR_SET_SIZE] = "abcdefghijklmnopqrstuvwxyz";
int counter = 0;

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

// Global variable for found state
struct SharedState {
    static std::atomic<bool> found;
};
std::atomic<bool> SharedState::found{false};

void brute_force_letters(const std::string tar_hashed, const std::string salt, 
                        std::string perm_curr, const std::string perm_end, 
                        const std::vector<int> start_pass, int depth = 1, 
                        bool lock = true) {
    // Check shared found flag
    if (SharedState::found) return;

    // Base case
    if (depth == LETTERS) {
        std::string full_salt = "$6$" + salt;
        std::cout << perm_curr.c_str() << std::endl;
        std::string curr_hashed = crypt(perm_curr.c_str(), full_salt.c_str()); // hash_password(perm_curr, salt);
        if (curr_hashed.compare(tar_hashed) == 0) {
            std::cout << "Found the pass !: " << perm_curr << '\n';
            std::cout << "Hashed: " << curr_hashed << std::endl;
            SharedState::found = true;
            return;
        }

        if (counter % PRINT_THRESHOLD == 0) {
            std::cout << "Counter: " << counter << std::endl;
            std::cout << "Combination: " << perm_curr << std::endl;
            std::cout << "Hashed: " << curr_hashed << std::endl;
        }

        if (perm_curr == perm_end) {
            std::cout << "Break Combination: " << perm_curr << std::endl;
            return;
        }
        counter++;
        return;
    } else if (depth < LETTERS) {
        int init_idx = 0;
        if (lock) init_idx = start_pass[depth];
        for (int i = init_idx; i < CHAR_SET_SIZE; i++) {
            char tar_char = char_set[i];
            std::string new_perm = perm_curr + tar_char;
            bool next_lock = lock;
            if (i > init_idx) next_lock = false;
            brute_force_letters(tar_hashed, salt, new_perm, perm_end, start_pass, 
                              depth + 1, next_lock);
        }
    }
}

bool tasks(char *tar_hashed, u_int16_t start_idx, u_int16_t end_idx, 
            u_long chunk_size, int cores) {

    // Extract salt
    std::string hash_str(tar_hashed);
    std::string salt;
    size_t salt_start = hash_str.find("$", 1);
    if (salt_start != std::string::npos) {
        size_t salt_end = hash_str.find("$", salt_start + 1);
        if (salt_end != std::string::npos) {
            salt = hash_str.substr(salt_start + 1, salt_end - salt_start - 1);
        }
    }

    // Spin multiple threads
    SharedState::found = false;
    cores = 1;
    int new_chunk_size = ceil((chunk_size * (end_idx - start_idx)) / cores);
    std::vector<u_long> thread_starts(cores);
    std::vector<u_long> thread_ends(cores);
    std::vector<std::thread> threads;
    for (int i = 0; i < cores; i++) {
        // Get the start and end of each thread
        thread_starts[i] = start_idx * chunk_size + i * new_chunk_size;
        thread_ends[i] = thread_starts[i] + ((i + 1) * new_chunk_size);

        // Convert int base 10 to array base 26
        std::vector<int> start_pass = base10_to_baseN(thread_starts[i], CHAR_SET_SIZE, LETTERS);
        std::vector<int> end_pass = base10_to_baseN(thread_ends[i], CHAR_SET_SIZE, LETTERS);

        // Convert array base 26 to string
        std::string perm_start = indices_to_string(start_pass);
        std::string perm_end = indices_to_string(end_pass);
        std::string perm_curr(1, perm_start[0]);    // current permutation eg. "a"

        std::cout << "\n------Thread-Summary-----\n" << std::endl;
        std::cout << "Core_id:" << i + 1 << std::endl;
        std::cout << "Perm_start:" << perm_start << std::endl;
        std::cout << "Perm_end:" << perm_end << std::endl;
        std::cout << "Chunk Size:" << new_chunk_size << std::endl;
        std::cout << "\n-------------------------\n" << std::endl;

        threads.emplace_back(brute_force_letters, 
                           hash_str, 
                           salt, 
                           perm_curr, 
                           perm_end, 
                           start_pass,
                           1,      // depth
                           true);  // lock
    }
    
    // Barrier here
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "\n-------End-Chunk-------\n" << std::endl;
    return SharedState::found;
}



#ifndef TASKS_H
#define TASKS_H

#include <string>

bool tasks(char *tar_hashed, u_int16_t start_idx, u_int16_t end_idx, 
            u_long chunk_size, int cores);
#endif
#include "get-specs.h"
#include <sys/statvfs.h>
#include <fstream>
#include <iostream>
#include <string>

int getTotalRAMInGiB() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    double totalRAMKiB = 0.0;

    if (meminfo.is_open()) {
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                // Extract the total memory in KiB from the line
                std::sscanf(line.c_str(), "MemTotal: %lf kB", &totalRAMKiB);
                break;
            }
        }
        meminfo.close();
    }

    // Convert from KiB to GiB
    int totalRAMGiB = totalRAMKiB / (1024.0 * 1024.0);
    return totalRAMGiB;
}

int getStorageInGiB(const std::string& path) {
    struct statvfs stat;

    if (statvfs(path.c_str(), &stat) != 0) {
        // Error handling if statvfs fails
        std::cerr << "Error: Unable to access file system information for path: " << path << std::endl;
        return -1;
    }

    // Calculate total storage in bytes
    unsigned long long totalBytes = stat.f_blocks * stat.f_frsize;

    // Convert to GiB
    int totalGiB = totalBytes / (1024.0 * 1024.0 * 1024.0);
    return totalGiB;
}

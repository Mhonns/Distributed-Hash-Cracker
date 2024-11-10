#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <fstream>
#include <sys/statvfs.h>
// Created Libs
#include "get-specs.h"
#include "sha512.h"

#define SUCCESS_MSG "SCMSG"
#define FOUND_MSG "FOUND"
#define FUALT_TOL 3

class UDPClient {
private:
    int sockfd;
    struct sockaddr_in servaddr;
    static const int MSG_SIZE = 6;  // Make sure this matches server's MSG_SIZE
    static const int TIMEOUT_SECONDS = 5;  // Timeout for receive

public:
    UDPClient(const char* server_ip, int port) {
        // Create socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        memset(&servaddr, 0, sizeof(servaddr));

        // Server configuration
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        servaddr.sin_addr.s_addr = inet_addr(server_ip);

        // Set receive timeout
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SECONDS;
        tv.tv_usec = 0;

        // 3 retries attempts
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                    throw std::runtime_error("Error setting timeout");
                    exit(0);
                }
            }
        }
    }

    bool sendAndReceive(const char* msg, uint16_t *start_index, uint16_t *end_index) {
        char buffer[MSG_SIZE];
        socklen_t len = sizeof(servaddr);

        // Send message
        ssize_t sent = sendto(sockfd, msg, MSG_SIZE, 0,
                            (const struct sockaddr *)&servaddr, sizeof(servaddr));
        
        if (sent < 0) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl;
            return false;
        }

        std::cout << "Sent message, waiting for response..." << std::endl;

        // Receive response
        ssize_t n = recvfrom(sockfd, buffer, MSG_SIZE, MSG_WAITALL,
                            (struct sockaddr *)&servaddr, &len);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Timeout waiting for response" << std::endl;
            } else {
                std::cerr << "Error receiving: " << strerror(errno) << std::endl;
            }
            return false;
        }

        // Check whether the success message was sent
        if (n == MSG_SIZE) {
            if (memcmp(buffer, SUCCESS_MSG, strlen(SUCCESS_MSG)) == 0) {
                std::cout << "Received: SUCCESS" << std::endl;
                return true;
            } else {
                std::cout << "Received: FAIL" << std::endl;
                return false;
            }
        } else {
            std::cout << "Received wrong message size: " << n << " bytes" << std::endl;
            return false;
        }
    }

    ~UDPClient() {
        close(sockfd);
    }
};

bool tasks(char *salt, int start_idx, int end_idx, char *char_set) {
    // TODO brute force run sha512 and write logs.txt if found return true
    
    return false;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Usages: bin_file hashed_value host_ip port chunk_size" << std::endl;
        return 1;
    }
    char *hashed = argv[1];
    char *ipv4 = argv[2];
    int port = atoi(argv[3]);
    u_long chunk_size = atol(argv[4]);
    bool found = false;

    // For looping
    uint16_t start_index = 0;
    uint16_t end_index = 0;

    // Register state For Linux only
    char msg[6] = "Z0000";
    msg[1] = sysconf(_SC_NPROCESSORS_ONLN);
    msg[2] = getTotalRAMInGiB();
    msg[3] = getStorageInGiB("/");
    UDPClient client(ipv4, port);
    try {
        client.sendAndReceive(msg, &start_index, &end_index);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // TODO Loop and start hashsed
    char char_set[27] = "abcdefghijklmnopqrstuvwxyz";
    while (found == false) {
        sleep(5);
        msg[0] = 'A';
        try {
            client.sendAndReceive(msg, &start_index, &end_index);
            found = tasks(hashed, start_index, end_index, char_set);

            // Found the password
            if (found == true) {
                sleep(2);
                msg[0] = 'F';
                std::cout << "Send out found" <<std::endl;
                try {
                    client.sendAndReceive(msg, &start_index, &end_index);
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                    return 1;
                }
                return 0;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
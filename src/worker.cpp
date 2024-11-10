#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                    throw std::runtime_error("Error setting timeout");
                }
            }
        }
    }

    bool sendAndReceive(const char* msg) {
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

        if (n == MSG_SIZE) {
            std::cout << "Server response: ";
            for (int i = 0; i < MSG_SIZE; i++) {
                printf("%02X ", (unsigned char)buffer[i]);
            }
            std::cout << std::endl;
            return true;
        } else {
            std::cout << "Received wrong message size: " << n << " bytes" << std::endl;
            return false;
        }
    }

    ~UDPClient() {
        close(sockfd);
    }
};

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Usages: bin_file hashed_value host_ip port chunk_size" << std::endl;
        return 1;
    }
    char *hashed = argv[1];
    char *ipv4 = argv[2];
    int port = atoi(argv[3]);
    int chunk_size = atoi(argv[4]);
    char start_msg[6] = "ZNOHW";
    UDPClient client(ipv4, port);
    try {
        client.sendAndReceive(start_msg);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    start_msg[0] = 'B';
    try {
        client.sendAndReceive(start_msg);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
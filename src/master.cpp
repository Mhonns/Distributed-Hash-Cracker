#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <fcntl.h> 
#include <fstream>
#include <cmath>

#define STATUS_STATE 'Z'
#define ASSIGN_STATE 'A'
#define FOUND_STATE 'F'
#define SUCCESS_MSG "SCMSG"
#define FOUND_MSG "FOUND"
#define FAIL_MSG "FLMSG"

int chunk_size = 0;
int chunk_index = 0;
int chunk_left = 1;
int found = false;

class UDPServer {
private:
    int sockfd;
    struct sockaddr_in servaddr;
    static const int MSG_SIZE = 6;
    std::mutex clientsMutex;
    std::map<std::string, struct sockaddr_in> clients; // Store client addresses

    // Helper function to get client identifier
    std::string getClientIdentifier(const struct sockaddr_in& addr) {
        char buff[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), buff, INET_ADDRSTRLEN);
        return std::string(buff);
    }

    // Handle individual client message
    void handleClient(const char* buffer, const struct sockaddr_in& client_addr, socklen_t len) {
        std::string clientId = getClientIdentifier(client_addr);
        {
            std::lock_guard<std::mutex> lock(clientsMutex); // Lock
            clients[clientId] = client_addr; // Store or update client information
        }
        const char *ret_val = state_handler(buffer, clientId);
        std::cout << "Ret val: " << ret_val << std::endl;

        // Ending condition
        if (strcmp(ret_val, FOUND_MSG) == 0) {
            std::cout << FOUND_MSG << std::endl;
            std::cout << "Done, Password in: " << clientId << std::endl;
            const std::string filename = "password_in.txt";
            std::ofstream outFile(filename);
            if (outFile.is_open())  {
                outFile << clientId;
                outFile.close();
            } else 
                std::cerr << "Error: Could not open file for writing." << std::endl;
            found = true;
            exit(0);
        }
        else if (chunk_left <= 0) {
            std::cout << "No password found" << std::endl;
            exit(0);
        }

        // Send response back to specific client
        sendto(sockfd, ret_val, MSG_SIZE, 0,
               (const struct sockaddr *)&client_addr, len);
        
        std::cout << "Response sent to client " << clientId << std::endl;
    }

    const char *state_handler(const char* buffer, std::string clientId) {
        char state = buffer[0];
        switch (state) {
            case (STATUS_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "\n1. Status Client: " << clientId << std::endl;
                std::cout << "Got CPU: " << int(buffer[1]) << "cores" << std::endl;
                std::cout << "Got RAM: " << int(buffer[2]) << "GiB" << std::endl;
                std::cout << "Got Storage: " << int(buffer[3]) << "GiB" << std::endl;
                std::cout << "Prepared Chunk Size: " << chunk_size << "B" << std::endl;
                return SUCCESS_MSG;
            }
            case (ASSIGN_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                uint16_t start_index = chunk_index++;
                uint16_t end_index = chunk_index;
                std::cout << "\n2. Assign task: " << clientId << std::endl;
                std::cout << "Start index: " << start_index << std::endl;
                std::cout << "End index: " << end_index << std::endl;
                chunk_left--;
                static char ret_msg[6];  // 'A' + 2 bytes for start + 2 bytes for end + null terminator
                ret_msg[0] = 'A';
                memcpy(&ret_msg[1], &start_index, sizeof(uint16_t));
                memcpy(&ret_msg[3], &end_index, sizeof(uint16_t));
                ret_msg[5] = '\0';  // Null terminator
                return ret_msg;
            }
            case (FOUND_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "\n3. Finish state from client " << clientId << std::endl;
                return FOUND_MSG;
            }
        }
        return FAIL_MSG;
    }

public:
    UDPServer(int port) {
        // Create socket with UDP
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        // Set socket options for reuse
        int opt = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("setsockopt failed");
        }

        memset(&servaddr, 0, sizeof(servaddr));

        // Server configuration
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);

        // Bind socket
        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        // Set socket to non-blocking mode
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }

    void receiveAndRespond() {
        char buffer[MSG_SIZE];
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        std::vector<std::thread> clientThreads;

        while (!found) {
            memset(buffer, 0, MSG_SIZE);
            
            ssize_t n = recvfrom(sockfd, buffer, MSG_SIZE, MSG_WAITALL,
                                (struct sockaddr *)&client_addr, &len);

            if (n == MSG_SIZE) {
                // Create a new thread to handle this client
                clientThreads.emplace_back(&UDPServer::handleClient, this, buffer, client_addr, len);
                
                // Clean up completed threads
                auto it = clientThreads.begin();
                while (it != clientThreads.end()) {
                    if (it->joinable()) {
                        it->join();
                        it = clientThreads.erase(it);
                    } else {
                        ++it;
                    }
                }
            }

            // Small sleep to prevent CPU overload
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    // Get current number of connected clients
    size_t getClientCount() {
        std::lock_guard<std::mutex> lock(clientsMutex);
        return clients.size();
    }

    ~UDPServer() {
        close(sockfd);
    }
};

int main(int argc, char *argv[]) {
    // Get the user input
    if (argc < 3) {
        std::cout << "Usages: bin_file port chunk_size hashed_value" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    chunk_size = atoi(argv[2]);
    char *hashed_pwd = argv[3];

    // Calculate all possible chunk
    chunk_left = pow(26, 7) / chunk_size;
    std::cout << "Chunk left: " << chunk_left << std::endl;

    // Connect to the network
    try {
        UDPServer server(port);
        std::cout << "Server listening on port " << port << "..." << std::endl;
        server.receiveAndRespond();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

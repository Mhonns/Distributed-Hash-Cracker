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

#define REG_STATE 'Z'
#define STATUS_STATE 'B'
#define ASSIGN_STATE 'C'
#define FINISH_STATE 'D'

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
        return std::string(buff); // + ":" + std::to_string(ntohs(addr.sin_port));
    }

    // Handle individual client message
    void handleClient(const char* buffer, const struct sockaddr_in& client_addr, socklen_t len) {
        std::string clientId = getClientIdentifier(client_addr);
        {
            std::lock_guard<std::mutex> lock(clientsMutex); // Lock
            clients[clientId] = client_addr; // Store or update client information
        }

        char state = buffer[0];
        switch (state) {
            case (REG_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "Register state from client " << clientId << std::endl;
                break;
            }
            case (STATUS_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "Status state from client " << clientId << std::endl;
                break;
            }
            case (ASSIGN_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "Assign state from client " << clientId << std::endl;
                break;
            }
            case (FINISH_STATE): {
                std::lock_guard<std::mutex> lock(clientsMutex);
                std::cout << "Finish state from client " << clientId << std::endl;
                break;
            }
        }

        // Send response back to specific client
        sendto(sockfd, buffer, MSG_SIZE, 0,
               (const struct sockaddr *)&client_addr, len);
        
        std::cout << "Response sent to client " << clientId << std::endl;
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

        while (true) {
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
    int chunk_size = atoi(argv[2]);
    char *hashed_pwd = argv[3];

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

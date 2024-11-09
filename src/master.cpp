#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

class UDPServer {
private:
    int sockfd;
    struct sockaddr_in servaddr, clientaddr;
    static const int MSG_SIZE = 5;

public:
    UDPServer(int port) {
        // Create socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&clientaddr, 0, sizeof(clientaddr));

        // Server configuration
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);

        // Bind socket
        if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            throw std::runtime_error("Bind failed");
        }
    }

    void receiveAndRespond() {
        char buffer[MSG_SIZE];
        socklen_t len = sizeof(clientaddr);

        while (true) {
            ssize_t n = recvfrom(sockfd, buffer, MSG_SIZE, MSG_WAITALL,
                                (struct sockaddr *)&clientaddr, &len);
            
            if (n == MSG_SIZE) {
                std::cout << "Received: ";
                for (int i = 0; i < MSG_SIZE; i++) {
                    printf("%02X ", (unsigned char)buffer[i]);
                }
                std::cout << std::endl;

                // Echo back
                sendto(sockfd, buffer, MSG_SIZE, 0,
                      (const struct sockaddr *)&clientaddr, len);
            }
        }
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

    // Connect to the network
    try {
        UDPServer server(8080);
        std::cout << "Server listening on port " << std::stoi(argv[1]) << "..." << std::endl;
        server.receiveAndRespond();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

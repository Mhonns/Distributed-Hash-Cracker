#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IPV4 "192.168.64.211"
#define PORT 30000

class UDPClient {
private:
    int sockfd;
    struct sockaddr_in servaddr;
    static const int MSG_SIZE = 5;

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
    }

    void sendAndReceive(const char* msg) {
        char buffer[MSG_SIZE];
        socklen_t len = sizeof(servaddr);

        // Send message
        sendto(sockfd, msg, MSG_SIZE, 0,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // Receive response
        ssize_t n = recvfrom(sockfd, buffer, MSG_SIZE, MSG_WAITALL,
                            (struct sockaddr *)&servaddr, &len);
        
        if (n == MSG_SIZE) {
            std::cout << "Server response: ";
            for (int i = 0; i < MSG_SIZE; i++) {
                printf("%02X ", (unsigned char)buffer[i]);
            }
            std::cout << std::endl;
        }
    }

    ~UDPClient() {
        close(sockfd);
    }
};

int send_msg(char *msg) {
    try {
        UDPClient client(IPV4, 8080);
        client.sendAndReceive(msg);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int main() {
    char start_msg[5] = {'Z', 'N', 'O', 'H', 'W'};
    int ret_val = send_msg(start_msg);
    if (ret_val == 0) {
        return 0;
    }
    return 1;
}
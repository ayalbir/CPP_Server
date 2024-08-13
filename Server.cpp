#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>

// Function to handle communication with the client
void handle_client(int client_sock) {
    char buffer[4096];
    int expected_data_len = sizeof(buffer);

    while (true) {
        // Initialize the buffer with null characters
        memset(buffer, 0, expected_data_len);

        int read_bytes = recv(client_sock, buffer, expected_data_len, 0);
        if (read_bytes == 0) {
            std::cout << "Connection closed by client" << std::endl;
            break;
        } else if (read_bytes < 0) {
            perror("Error receiving data");
            break;
        } else {
            std::cout << "Received from client: " << buffer << std::endl;

            int sent_bytes = send(client_sock, buffer, read_bytes, 0);
            if (sent_bytes < 0) {
                perror("Error sending to client");
                break;
            }
        }
    }

    close(client_sock);  // Close the client socket when done
}


int main() {
    const int server_port = 5555;

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        return 1;
    }

    // Initialize socket structure
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(server_port);

    // Bind the socket
    if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("Error binding socket");
        close(sock);
        return 1;
    }

    // Listen to the socket
    if (listen(sock, 5) < 0) {
        perror("Error listening to socket");
        close(sock);
        return 1;
    }

    std::cout << "Server is listening on port " << server_port << std::endl;

    while (true) {
        // Accept client connection
        struct sockaddr_in client_sin;
        unsigned int addr_len = sizeof(client_sin);
        int client_sock = accept(sock, (struct sockaddr*)&client_sin, &addr_len);
        if (client_sock < 0) {
            perror("Error accepting client");
            close(sock);
            return 1;
        }

        // Create a new thread to handle the client
        std::thread(handle_client, client_sock).detach();
    }

    close(sock);  // Close the server socket when done

    return 0;
}

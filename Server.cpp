#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;

int main() {
    const int server_port = 5555;

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("error creating socket");
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
        perror("error binding socket");
        close(sock);
        return 1;
    }

    // Listen to the socket
    if (listen(sock, 5) < 0) {
        perror("error listening to socket");
        close(sock);
        return 1;
    }

    // Accept client connection
    struct sockaddr_in client_sin;
    unsigned int addr_len = sizeof(client_sin);
    int client_sock = accept(sock, (struct sockaddr*)&client_sin, &addr_len);
    if (client_sock < 0) {
        perror("error accepting client");
        close(sock);
        return 1;
    }

    // Receive data from client
    char buffer[4096];
    int expected_data_len = sizeof(buffer);
    int read_bytes = recv(client_sock, buffer, expected_data_len, 0);
    if (read_bytes == 0) {
        cout << "connection is closed by client" << endl;
    } else if (read_bytes < 0) {
        perror("error receiving data");
    } else {
        cout << "Received from client: " << buffer << endl;
    }

    // Send data back to client (echo)
    int sent_bytes = send(client_sock, buffer, read_bytes, 0);
    if (sent_bytes < 0) {
        perror("error sending to client");
    }

    // Close sockets
    close(client_sock);
    close(sock);

    return 0;
}

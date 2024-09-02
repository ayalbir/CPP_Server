#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

// Data structures to track video views
std::map<std::string, std::vector<std::string>> videoWatchMap; // Maps videoId to a list of userIds
std::map<std::string, std::vector<std::string>> userWatchMap;  // Maps userId to a list of videoIds
// Function to add a user's view of a video
void addView(const std::string &userId, const std::string &videoId) {
    userWatchMap[userId].push_back(videoId);
    videoWatchMap[videoId].push_back(userId);
}

// Function to get recommended videos based on the user's watched videos
std::vector<std::string> getRecommendations(const std::string &userId) {
    std::vector<std::string> watchedVideos = userWatchMap[userId];
    std::map<std::string, int> videoScores;
    for (const std::string &videoId: watchedVideos) {
        for (const std::string &otherUserId: videoWatchMap[videoId]) {
            if (otherUserId != userId) {
                for (const std::string &otherVideoId: userWatchMap[otherUserId]) {
                    if (std::find(watchedVideos.begin(), watchedVideos.end(), otherVideoId) == watchedVideos.end()) {
                        videoScores[otherVideoId]++;
                    }
                }
            }
        }
    }
    std::vector<std::pair<std::string, int>> sortedVideos(videoScores.begin(), videoScores.end());
    std::sort(sortedVideos.begin(), sortedVideos.end(), [](const auto &a, const auto &b) {
        return b.second < a.second;
    });
    std::vector<std::string> recommendedVideos;
    for (const auto &videoPair: sortedVideos) {
        recommendedVideos.push_back(videoPair.first);
    }
    return recommendedVideos;
}

// Function to handle communication with the client
void handle_client(int client_sock) {
    char buffer[4096];
    int expected_data_len = sizeof(buffer);

    while (true) {
        // Initialize the buffer with null characters
        memset(buffer, 0, expected_data_len);

        int read_bytes = recv(client_sock, buffer, expected_data_len, 0);
        if (read_bytes == 0) {
            //std::cout << "Connection closed by client" << std::endl;
            break;
        } else if (read_bytes < 0) {
            perror("Error receiving data");
            break;
        } else {
            std::string input(buffer);
            std::istringstream stream(input);
            std::string command;
            stream >> command;

            if (command == "UPDATE_VIEW") {
                std::string userId, videoId;
                stream >> userId >> videoId;

                addView(userId, videoId);

                std::string response = "Added view: " + userId + " watched " + videoId;
                int sent_bytes = send(client_sock, response.c_str(), response.size(), 0);
                if (sent_bytes < 0) {
                    perror("Error sending to client");
                }
            } else if (command == "GET_RECOMMENDATIONS") {
                std::string userId;
                stream >> userId;

                std::vector<std::string> recommendations = getRecommendations(userId);
                std::string response;
                for (const std::string &videoId: recommendations) {
                    response += videoId + " ";
                }
                send(client_sock, response.c_str(), response.size(), 0);
            } else {
                std::string response = "Unknown command\n";
                send(client_sock, response.c_str(), response.size(), 0);
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
    if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
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
        int client_sock = accept(sock, (struct sockaddr *) &client_sin, &addr_len);
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

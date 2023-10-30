#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 12345;
const char* SERVER_IP = "127.0.0.1"; // Change to the IP of your server

// Structure to hold received messages
struct message_t {
    std::string sender;
    std::string content;
};

std::vector<message_t> received_messages;
pthread_mutex_t mutex;

// Function to send a message
void* send_thread(void* arg) {

    return nullptr;
}

// Function to receive messages
void* receive_thread(void* arg) {
    
    return nullptr;
}

int main() {
    pthread_t sendThread, receiveThread;

    // Initialize the mutex for thread synchronization
    pthread_mutex_init(&mutex, nullptr);

    // Create the send and receive threads
    pthread_create(&sendThread, nullptr, send_thread, nullptr);
    pthread_create(&receiveThread, nullptr, receive_thread, nullptr);

    // Wait for the threads to finish (you can implement a termination condition)
    pthread_join(sendThread, nullptr);
    pthread_join(receiveThread, nullptr);

    // Cleanup
    pthread_mutex_destroy(&mutex);

    return 0;
}

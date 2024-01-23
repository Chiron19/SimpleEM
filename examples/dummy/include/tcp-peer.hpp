#ifndef TCP_PEER_HPP
#define TCP_PEER_HPP

#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network-helper.hpp"

/**
 * @brief Control the TCP peers (send and receive) in two threads
 * 
 * Class responsible for the cooperation bewteen TCP peers. It creates and starts two threads simultaneously, one for sending and one for receiving. The received messages are stored in a stack buffer, and the sending messages are popped from the stack buffer. The threads are synchronized by a mutex and a condition variable.
 */
class TCPpeer {

protected:

    int em_id;
    NetworkHelper& net_send;
    NetworkHelper& net_recv;

public: 

    /**
     * @brief Construct a new TCPpeer object
     * 
     * @param em_id The em_id of this peer
     * @param net_send The network helper for sending
     * @param net_recv The network helper for receiving
    */
    TCPpeer(int em_id, NetworkHelper& net_send, NetworkHelper& net_recv): em_id(em_id), net_send(net_send), net_recv(net_recv) {
        // std::cout << "[tcp-peer] init net_send " << net_send.em_id << '/' << net_send.procs << std::endl;
        // std::cout << "[tcp-peer] init net_recv " << net_send.em_id << '/' << net_send.procs << std::endl;
    }

    std::vector<message_t> messages_to_send; ///< Message stack buffer
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::string extension_str = ".svg"; ///< For large file test

    /**
     * @brief Create threads for both sender and receiver
     * 
     * @param obj The TCPpeer object, must initialized first.
    */
    void tcp_thread(TCPpeer* obj) {

        /*****For ping-pong packet test********/
        // if (em_id == 0) {
        //     for (int i = 1; i < net_send.procs; ++i) {
        //         messages_to_send.push_back({i, "pong"}); // For buffer string test
        //     }
        // }
        /*****For large file transfer test*****/
        if (em_id == 0) messages_to_send.push_back({1, "epfl-logo.svg"}); // For large file test
        /*************************************/

        // Initialize the mutex for thread synchronization
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr); 

        // Create the send and receive threads
        pthread_create(&sendThread, nullptr, send_thread_wrapper, obj);
        // std::cout << "[tcp-peer] pthread_create sendThread" << std::endl;
        pthread_create(&recvThread, nullptr, recv_thread_wrapper, obj);
        // std::cout << "[tcp-peer] pthread_create recvThread" << std::endl;
        
        // Wait for the threads to finish (you can implement a termination condition)
        pthread_join(sendThread, &sendThread_return);
        // std::cout << "[tcp-peer] pthread_join sendThread" << std::endl;
        pthread_join(recvThread, &recvThread_return);
        // std::cout << "[tcp-peer] pthread_join recvThread" << std::endl;
        
        // int result = *(int *)sendThread_return;

        // Cleanup
        pthread_mutex_destroy(&mutex);
    }

    /**
     * @brief Psuedo broadcast function (send to all other em_id except itself)
     * 
     * @param target_em_id The target em id
     * @param message The message to send
    */
    void broadcast(int em_id, const std::string& message) {
        for (int other_id = 0; other_id < net_send.procs; ++other_id) {
            if (em_id == other_id) continue;
            if (net_send.send_tcp(other_id, message) < 0) {
                std::cout << "[tcp-peer] " << em_id << "->" << other_id << " FAIL" << std::endl;
            }
            else 
            std::cout << "[tcp-peer] " << em_id << "->" << other_id << " (" << message << ")" << std::endl;
        }
    }

    /**
     * @brief Force receive a message
     * 
     * @return The message received, or -1 if failed.
    */
    message_t force_receive() {
        while(true) {
            message_t mes = net_recv.receive_tcp();
            
            if (mes.first >= 0) {
                std::cout << "[tcp-peer] recv from " << mes.first << " : " << mes.second << std::endl;
                return mes;
            }
            else {
                std::cout << "[tcp-peer] recv fail " << std::endl;
                return {-1, ""};
            }
        }
    }

private:
    pthread_t sendThread, recvThread;
    void* sendThread_return;
    void* recvThread_return;

    // Static wrapper function to call the member function
    static void* send_thread_wrapper(void* obj) {
        return static_cast<TCPpeer*>(obj)->send_thread(nullptr);
    }

    // Static wrapper function to call the member function
    static void* recv_thread_wrapper(void* obj) {
        return static_cast<TCPpeer*>(obj)->recv_thread(nullptr);
    }

    /**
     * @brief The send thread function
     * 
     * @param arg The argument passed to the thread, must instantiated first.
    */
    void* send_thread(void* arg) {
        // std::cout << "[tcp-peer] send_thread" << std::endl;
        int *result = static_cast<int*>(malloc(sizeof(int)));
        *result = 0;
        
        while (1) {
            // Lock the mutex to safely access messages_to_send
            pthread_mutex_lock(&mutex);
            while (messages_to_send.empty()) {
                pthread_cond_wait(&cond, &mutex);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                printf("[tcp-peer] send_thread waiting...\n");
            }
            message_t mes = messages_to_send.back();
            messages_to_send.pop_back();
            pthread_mutex_unlock(&mutex); // Unlock the mutex

            int target_em_id = mes.first;
            std::string message = mes.second;

            if (target_em_id >= 0) {
                
                /*****For ping-pong packet test********/
                // if (message == "ping") message = "pong"; else message = "ping"; // For sending message string test

                // std::cout << "[tcp-peer] send_thread sending:" << em_id << "->" << target_em_id << " " << message << std::endl;

                // while (net_send.send_tcp(target_em_id, message) < 0);// Send the message

                /*****For large file transfer test*****/
                // For sending large file test
                std::cout << "[tcp-peer] send_thread sending:" << em_id << "->" << target_em_id << " " << message << std::endl;

                // Send the file
                while (net_send.send_tcp(target_em_id, message, 1) < 0);

                /*************************************/
            }
        }

        sleep(1); 
        *result = 1;
        std::cout << "[tcp-peer] send_thread return " << *result << std::endl;
        return result;
    }

    /**
     * @brief The receive thread function
     * 
     * @param arg The argument passed to the thread, must instantiated first.
    */
    void* recv_thread(void* arg) {
        // std::cout << "[tcp-peer] recv_thread" << std::endl;
        int *result = static_cast<int*>(malloc(sizeof(int)));
        *result = 0;

        srand(time(NULL));

        while (1) {

            /*****For ping-pong packet test********/
            // message_t mes = force_receive(); // For receiving text
            
            /*****For large file transfer test*****/
            // For receiving large file, rename the file to time + random number + extension
            std::string filePath = net_recv.getLocalTime() + "-" + std::to_string(rand()%10000) + extension_str;
            message_t mes;
            // std::cout << "[tcp-peer] recv filePath: " << filePath << std::endl;
            
            do {
                mes = net_recv.receive_tcp(filePath, 1); // For receiving large file
            } while (mes.first < 0);

            /*************************************/
        
            std::cout << "[tcp-peer] " << em_id << " GOT FROM " << mes.first << " MESSAGE: " << mes.second << std::endl;

            // Lock the mutex to safely access messages_to_send
            pthread_mutex_lock(&mutex);
            messages_to_send.push_back(mes);
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mutex);
        }  
        
        sleep(1);
        *result = 1;
        std::cout << "[tcp-peer] recv_thread return " << *result << std::endl;
        return result;
    }
};


#endif /* TCP_PEER_HPP */
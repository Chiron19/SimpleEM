#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network-helper.hpp"

class TCPpeer {

protected:

    int em_id;
    NetworkHelper& net_send;
    NetworkHelper& net_recv;

public: 

    TCPpeer(int em_id, NetworkHelper& net_send, NetworkHelper& net_recv): em_id(em_id), net_send(net_send), net_recv(net_recv) {
        std::cout << "[tcp-peer] init net_send " << net_send.procs << ' ' << net_send.em_id << std::endl;
        std::cout << "[tcp-peer] init net_recv " << net_recv.procs << ' ' << net_recv.em_id << std::endl;
    }

    std::vector<message_t> received_messages;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex1;
    pthread_mutex_t mutex2;

    // create thread for both sender and receiver
    void tcp_thread() {
        
        // Initialize the mutex for thread synchronization
        pthread_mutex_init(&mutex, nullptr);
        pthread_mutex_init(&mutex1, nullptr);
        pthread_mutex_init(&mutex2, nullptr);

        std::cout << "[tcp-peer] pthread_mutex_init" << std::endl;

        if (em_id == 0) received_messages.push_back({1, "pong"});
        std::cout << "[tcp-peer] received_messages.push_back({1, 'pong'})" << std::endl;
        std::cout << "[tcp-peer] received_messages:" << received_messages.size() << std::endl;

        // Create the send and receive threads
        pthread_create(&sendThread, nullptr, send_thread_wrapper, nullptr);
        std::cout << "[tcp-peer] pthread_create sendThread" << std::endl;

        pthread_create(&recvThread, nullptr, recv_thread_wrapper, nullptr);
        std::cout << "[tcp-peer] pthread_create recvThread" << std::endl;

        // Wait for the threads to finish (you can implement a termination condition)
        pthread_join(sendThread, &sendThread_return);
        std::cout << "[tcp-peer] pthread_join(sendThread, &sendThread_return);" << std::endl;
        pthread_join(recvThread, &recvThread_return);
        std::cout << "[tcp-peer] pthread_join(recvThread, &recvThread_return);" << std::endl;

        // int result = *(int *)sendThread_return;

        // Cleanup
        pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);
    }

    void broadcast(int em_id, const std::string& message) {
        for (int other_id = 0; other_id < net_send.procs; ++other_id) {
            // net.send(other_id, message);
            if (em_id == other_id) continue;
            if (net_send.send_tcp(other_id, message) < 0) {
                // 
                std::cout << "[tcp-peer] " << em_id << "->" << other_id << " FAIL" << std::endl;
            }
            else 
            std::cout << "[tcp-peer] " << em_id << "->" << other_id << " (" << message << ")" << std::endl;
        }
    }

    message_t force_receive() {
        while(true) {
            std::cout << "[tcp-peer] recv ing " << std::endl;
            std::cout << "ERROR OCCURS" << net_recv.procs << ' ' << net_recv.em_id << std::endl;
            message_t mes = net_recv.receive_tcp();
            std::cout << "[tcp-peer] recv mes: " << mes.first << " " << mes.second << std::endl;
            
            if (mes.first >= 0) {
                std::cout << "[tcp-peer] recv from " << mes.first << " : " << mes.second << std::endl;
                return mes;
            }
            else {
                std::cout << "[tcp-peer] recv fail " << std::endl;
            }
        }
    }

private:
    pthread_t sendThread, recvThread;
    void* sendThread_return;
    void* recvThread_return;

    // Static wrapper function to call the member function
    static void* send_thread_wrapper(void* obj) {
        TCPpeer* peer = static_cast<TCPpeer*>(obj);
        return peer->send_thread(nullptr);
    }

    // Static wrapper function to call the member function
    static void* recv_thread_wrapper(void* obj) {
        TCPpeer* peer = static_cast<TCPpeer*>(obj);
        return peer->recv_thread(nullptr);
    }

    void* send_thread(void* arg) {
        std::cout << "[tcp-peer] send_thread" << std::endl;
        int *result = static_cast<int*>(malloc(sizeof(int)));
        *result = 0;
        
        std::cout << "[tcp-peer] send_thread result created" << std::endl;
        // Lock the mutex to safely access received_messages
        pthread_mutex_lock(&mutex);
        std::cout << "[tcp-peer] send_thread mutex locked" << std::endl;
        std::cout << "[tcp-peer] send_thread received_messages:" << received_messages.size() << std::endl;
        if (!received_messages.empty()) {
            message_t mes = received_messages.back();
            received_messages.pop_back();
            pthread_mutex_unlock(&mutex); // Unlock the mutex

            int target_em_id = mes.first;
            std::string message = mes.second;

            if (target_em_id >= 0) {
                if (message == "ping") message = "pong";
                else message = "ping";

                std::cout << "[tcp-peer] send_thread sending:" << target_em_id << " " << message << std::endl;

                pthread_mutex_lock(&mutex2);
                // Send the message
                while (net_send.send_tcp(target_em_id, message) < 0);
                pthread_mutex_unlock(&mutex2);
            }

            *result = 1;
        } else {
            // If received_messages is empty, unlock the mutex
            pthread_mutex_unlock(&mutex);
        }
        
        std::cout << "[tcp-peer] send_thread return " << *result << std::endl;
        return result;
    }

    void* recv_thread(void* arg) {
        std::cout << "[tcp-peer] recv_thread" << std::endl;
        // pthread_mutex_lock(&mutex1);
        message_t mes = force_receive();
        // pthread_mutex_unlock(&mutex1);
        Logger::print_string_safe(
            std::to_string(em_id) + " GOT FROM " + std::to_string(mes.first) + " MESSAGE: " + mes.second + "\n");

        // Lock the mutex to safely access received_messages
        pthread_mutex_lock(&mutex);
        std::cout << "[tcp-peer] recv_thread mutex locked" << std::endl;
        received_messages.push_back(mes);

        // Unlock the mutex after adding the message to received_messages
        pthread_mutex_unlock(&mutex);

        int *result = static_cast<int*>(malloc(sizeof(int)));
        *result = 1;
        std::cout << "[tcp-peer] recv_thread return " << *result << std::endl;
        return result;
    }
};

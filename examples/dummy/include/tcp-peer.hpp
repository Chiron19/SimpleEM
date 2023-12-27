#pragma once

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
        std::cout << "[tcp-peer] init net_send " << net_send.em_id << '/' << net_send.procs << std::endl;
        std::cout << "[tcp-peer] init net_recv " << net_send.em_id << '/' << net_send.procs << std::endl;
        ack_message.assign(net_send.procs, 0);
    }

    std::vector<message_t> received_messages;
    std::vector<bool> ack_message;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_mutex_t mutex1;
    pthread_cond_t cond1;

    // create thread for both sender and receiver
    void tcp_thread(TCPpeer* obj) {
        
        // Initialize the mutex for thread synchronization
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);

        // if (em_id == 0) received_messages.push_back({1, "pong"}); // For buffer string test
        if (em_id == 0) received_messages.push_back({1, "test_img.png"}); // For large file test

        while (true) {
            // Create the send and receive threads
            pthread_create(&sendThread, nullptr, send_thread_wrapper, obj);
            std::cout << "[tcp-peer] pthread_create sendThread" << std::endl;

            pthread_create(&recvThread, nullptr, recv_thread_wrapper, obj);
            std::cout << "[tcp-peer] pthread_create recvThread" << std::endl;

            // Wait for the threads to finish (you can implement a termination condition)
            pthread_join(sendThread, &sendThread_return);
            std::cout << "[tcp-peer] pthread_join(sendThread, &sendThread_return);" << std::endl;

            pthread_join(recvThread, &recvThread_return);
            std::cout << "[tcp-peer] pthread_join(recvThread, &recvThread_return);" << std::endl;
        }
        // int result = *(int *)sendThread_return;

        // Cleanup
        pthread_mutex_destroy(&mutex);
    }

    void broadcast(int em_id, const std::string& message) {
        for (int other_id = 0; other_id < net_send.procs; ++other_id) {
            // net.send(other_id, message);
            if (em_id == other_id) continue;
            if (net_send.send_tcp(other_id, message) < 0) {
                std::cout << "[tcp-peer] " << em_id << "->" << other_id << " FAIL" << std::endl;
            }
            else 
            std::cout << "[tcp-peer] " << em_id << "->" << other_id << " (" << message << ")" << std::endl;
        }
    }

    message_t force_receive() {
        while(true) {
            // std::cout << "[tcp-peer] recving " << std::endl;
            message_t mes = net_recv.receive_tcp();
            // std::cout << "[tcp-peer] recv mes: " << mes.first << " " << mes.second << std::endl;
            
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
        return static_cast<TCPpeer*>(obj)->send_thread(nullptr);
    }

    // Static wrapper function to call the member function
    static void* recv_thread_wrapper(void* obj) {
        return static_cast<TCPpeer*>(obj)->recv_thread(nullptr);
    }

    void* send_thread(void* arg) {
        // std::cout << "[tcp-peer] send_thread" << std::endl;
        int *result = static_cast<int*>(malloc(sizeof(int)));
        *result = 0;
        
        // Lock the mutex to safely access received_messages
        pthread_mutex_lock(&mutex);

        while (received_messages.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        message_t mes = received_messages.back();
        received_messages.pop_back();
        pthread_mutex_unlock(&mutex); // Unlock the mutex

        int target_em_id = mes.first;
        std::string message = mes.second;

        if (target_em_id >= 0) {
            // // For sending message string test
            // if (message == "ping") message = "pong"; else message = "ping";

            // std::cout << "[tcp-peer] send_thread sending:" << em_id << "->" << target_em_id << " " << message << std::endl;

            // // Send the message
            // while (net_send.send_tcp(target_em_id, message) < 0);

            // For sending large file test
            std::cout << "[tcp-peer] send_thread sending:" << em_id << "->" << target_em_id << " " << message << std::endl;

            // Send the file
            while (net_send.send_tcp(target_em_id, message, 1) < 0);
        }

        sleep(1); 
        *result = 1;
        std::cout << "[tcp-peer] send_thread return " << *result << std::endl;
        return result;
    }

    void* recv_thread(void* arg) {
        // std::cout << "[tcp-peer] recv_thread" << std::endl;
        int *result = static_cast<int*>(malloc(sizeof(int)));
        
        // message_t mes = force_receive(); // For receiving text
        std::string filePath = net_recv.getLocalTime()+".png";
        message_t mes;
        do {
            mes = net_recv.receive_tcp(filePath, 1); // For receiving large file
        } while (mes.first < 0);
        // std::cout << "[tcp-peer] " << em_id << " GOT FROM " << mes.first << " MESSAGE: " << mes.second << std::endl;

        // Lock the mutex to safely access received_messages
        pthread_mutex_lock(&mutex);
        received_messages.push_back(mes);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        sleep(1);
        *result = 1;
        std::cout << "[tcp-peer] recv_thread return " << *result << std::endl;
        return result;
    }
};

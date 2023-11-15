#include <iostream>
#include <pthread.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network-helper.hpp"

class Test {

protected:

    int em_id;
    NetworkHelper& net_send;
    NetworkHelper& net_recv;
    int num_to_send;

public: 

    Test(int em_id, NetworkHelper& net_send_, NetworkHelper& net_recv_): em_id(em_id), net_send(net_send_), net_recv(net_recv_) {
        std::cout << "[tcp-peer] init net_send " << net_send.procs << ' ' << net_send.em_id << std::endl;
        std::cout << "[tcp-peer] init net_recv " << net_recv.procs << ' ' << net_recv.em_id << std::endl;
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    // create thread for both sender and receiver
    void tcp_thread(Test* obj) {
        
        // Initialize the mutex for thread synchronization
        pthread_mutex_init(&mutex, nullptr);
        std::cout << "[tcp-peer] pthread_mutex_init" << std::endl;

        pthread_cond_init(&cond, nullptr);
        // std::cout << "[tcp-peer] num_to_send " << num_to_send << std::endl;

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

        // int result = *(int *)sendThread_return;

        // Cleanup
        pthread_mutex_destroy(&mutex);
    }

    // Static wrapper function to call the member function
    static void* send_thread_wrapper(void* obj) {
        return static_cast<Test*>(obj)->send_thread(nullptr);
    }

    // Static wrapper function to call the member function
    static void* recv_thread_wrapper(void* obj) {
        return static_cast<Test*>(obj)->recv_thread(nullptr);
    }

private:
    pthread_t sendThread, recvThread;
    void* sendThread_return;
    void* recvThread_return;

    void* send_thread(void* arg) {
        for (int i = 0; i < net_send.procs; i++)
        {
            std::cout << "[test send] " << net_send.addresses[i].first.c_str() << ' ' << net_send.addresses[i].second <<  std::endl;
        }
        int temp = 0;
        while (true)
        {
            net_send.send_tcp(0, "hi");
            pthread_mutex_lock(&mutex);
            while (num_to_send == 0) {
                std::cout << "[tcp-peer] sent mutex waiting" << std::endl;
                pthread_cond_wait(&cond, &mutex);
                std::cout << "[tcp-peer] sent mutex finish waiting" << std::endl;
            }
            num_to_send --;
            pthread_mutex_unlock(&mutex);
            // std::cout << "[tcp-peer] sent " << ++ temp << std::endl;
            sleep(1);
        }
        // ...
    }

    void* recv_thread(void* arg) {
        for (int i = 0; i < net_recv.procs; i++)
        {
            std::cout << "[test recv] " << net_recv.addresses[i].first.c_str() << ' ' << net_recv.addresses[i].second <<  std::endl;
        }
        while (true)
        {
            message_t mes = net_recv.receive_tcp();
            std::cout << "[tcp-peer] recv: " << mes.second <<  std::endl;
            pthread_mutex_lock(&mutex);
                // std::cout << "[tcp-peer] recv mutex" << std::endl;
            num_to_send ++;
            pthread_cond_broadcast(&cond);
                // std::cout << "[tcp-peer] recv cond finish broadcast" << std::endl;
            pthread_mutex_unlock(&mutex);
        }
        // ...
    }
};

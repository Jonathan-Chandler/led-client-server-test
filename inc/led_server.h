#ifndef __LED_SERVER_H__
#define __LED_SERVER_H__
#include <atomic>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <future>

#include "led.h"
#include "led_network.h"

class Led_Server : public Led_Network
{
public:
    Led_Server(int port);
    ~Led_Server() override;

    void initialize();

    void start_server();
    void stop_server();
    bool get_server_is_running();

private:
    struct sockaddr_in server_addr;
    int server_port;
    std::atomic<bool> server_is_running;

    void bind_socket();
};

class Led_Server_Nonblocking
{
public:
    Led_Server_Nonblocking(int port);
    ~Led_Server_Nonblocking();

    void initialize();
    void start_server();
    void stop_server();

    bool get_server_is_running();
    int get_send_message_count();
    int get_receive_message_count();

private:
    Led_Server server;
    bool socket_initialized;
    std::future<void> server_thread_handle;
};

#endif // ifndef __Led_Server_H__


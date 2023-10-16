#ifndef __LED_SERVER_H__
#define __LED_SERVER_H__
#include "led.h"
#include <atomic>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "led_network.h"

class Led_Server : public Led_Network
{
public:
    Led_Server(int port);
    ~Led_Server() override;

    void bind_socket();
    void start_server();
    void stop_server();
    bool get_server_is_running();
    void close_socket();
    int get_valid_message_count();

private:
    struct sockaddr_in server_addr;
    int server_port;
    std::atomic<bool> server_is_running;
    std::atomic<int> valid_message_count;
};

#endif // ifndef __Led_Server_H__


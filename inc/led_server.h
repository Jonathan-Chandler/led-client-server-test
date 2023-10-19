#ifndef __LED_SERVER_H__
#define __LED_SERVER_H__
#include <atomic>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "led.h"
#include "led_network.h"

class Led_Server : public Led_Network
{
public:
    Led_Server(int port);
    ~Led_Server() override;
    int test();

    void bind_socket();
    void start_server();
    void stop_server();
    bool get_server_is_running();
    //void close_socket();


private:
    struct sockaddr_in server_addr;
    int server_port;
    std::atomic<bool> server_is_running;
};

#endif // ifndef __Led_Server_H__


#ifndef __LED_CLIENT_H__
#define __LED_CLIENT_H__
#include "led.h"
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>


class Led_Client
{
public:
    Led_Client(std::string ip_addr, int port);
    ~Led_Client();

    //void send(std::string ipv4_addr, std::unique_ptr<Led_Strip::ser_led_strip_t> serialized_leds);
    //void receive();

    void create_socket();
    void bind_socket();
    void send_socket();
    void close_socket();

private:
    struct sockaddr_in server_addr;
    std::string server_ip;
    int server_port;
    bool socket_initialized;
    int client_fd;
};

#endif // ifndef __LED_CLIENT_H__


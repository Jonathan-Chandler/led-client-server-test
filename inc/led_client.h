#ifndef __LED_CLIENT_H__
#define __LED_CLIENT_H__
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "led.h"
#include "led_network.h"

class Led_Client : public Led_Network
{
public:
    Led_Client(std::string ip_addr, int port);
    ~Led_Client() override;

    void initialize();
    void send(std::vector<uint8_t> &client_message);

private:
    struct sockaddr_in server_addr;
    std::string server_ip;
    int server_port;

    void bind_socket();
    void set_socket_timeout();
};

#endif // ifndef __LED_CLIENT_H__


#ifndef __LED_CLIENT_H__
#define __LED_CLIENT_H__
#include "led.h"
#include "led_network.h"
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>


class Led_Client : public Led_Network
{
public:
    Led_Client(std::string ip_addr, int port);
    ~Led_Client() override;

    //void send(std::string ipv4_addr, std::unique_ptr<Led_Strip::ser_led_strip_t> serialized_leds);
    //void receive();

    void create_socket();
    void bind_socket();
    //void send_socket();
    void close_socket();
    //void send_leds(std::vector<uint8_t> data);

    void send_test();

private:
    struct sockaddr_in server_addr;
    std::string server_ip;
    int server_port;

};

#endif // ifndef __LED_CLIENT_H__


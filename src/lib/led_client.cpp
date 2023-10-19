#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <errno.h>

#include "debug.h"
#include "led_client.h"
#include "led.h"

Led_Client::Led_Client(std::string ip, int port)
    : Led_Network()
    , server_ip(ip)
    , server_port(port)
{
}

Led_Client::~Led_Client()
{
}

void Led_Client::bind_socket()
{
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(server_port);

    // failed to get inet_addr from server_ip string
    if (server_addr.sin_addr.s_addr == INADDR_NONE)
    {
        std::ostringstream err_str;

        err_str << "Led_Client bind failed to convert server ip: " << server_ip;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    if (socket_fd < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Client bind received invalid socket descriptor: " << socket_fd;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // connect to server
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to connect to server: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Update socket init status
    socket_initialized = true;
    dbg_notice("Successfully bind socket");
}

void Led_Client::set_socket_timeout()
{  
    // timeout after 3 seconds if connect fails
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) 
    {
        std::ostringstream err_str;

        err_str << "Failed to set timeout: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    dbg_notice("set timeout to 3 seconds");
}

void Led_Client::send(std::vector<uint8_t> &client_message)
{
    Led_Strip response_leds(1);

    // send to server
    dbg_notice("send LEDs to server");
    dbg_verbose_print_vector(client_message);
    send_all(socket_fd, client_message);

    // get response from server
    dbg_notice("get server response");
    std::vector<uint8_t> response_data = receive_all(socket_fd);
    response_leds.set_leds_from_net_frame(response_data);
    response_leds.print_all_leds();

    dbg_notice("exit");
}

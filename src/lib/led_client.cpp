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
    , socket_initialized(false)
{
}

Led_Client::~Led_Client()
{
    close_socket();
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

    if (sock_fd < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Client bind received invalid socket descriptor: " << sock_fd;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // connect to server
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
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

void Led_Client::send_test()
{
    Led_Strip client_leds(3, 255, 0, 255);
    std::vector<uint8_t> client_message = client_leds.get_led_net_frame();

    //ssize_t bytes_rcvd;
    //const char *client_msg_data = "Message from client";
    //char *client_ptr;

    send_all(client_message);

#if 0
    // send client message to server
    if ((bytes_sent = send(sock_fd, &client_msg, sizeof(client_msg), 0)) != sizeof(client_msg))
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send: sent " << bytes_sent << " (expected " << sizeof(client_msg) << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Receive response from server
    if ((bytes_rcvd = recv(sock_fd, &server_msg, sizeof(server_msg), 0)) != sizeof(server_msg))
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to receive server message: receive " << bytes_rcvd << " (expected " << sizeof(server_msg) << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    if (server_msg.magic == LED_MSG_MAGIC)
    {
        std::ostringstream notice_str;
        notice_str << "Led_Client received data: " << server_msg.data;
        dbg_notice("%s", notice_str.str().c_str());
    }
    else
    {
        std::ostringstream err_str;
        err_str << "Led_Client received invalid magic value from server";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }
#endif
}

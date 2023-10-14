#include "debug.h"
#include "led_client.h"
#include "led_frame.h"
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <errno.h>

Led_Client::Led_Client(std::string ip, int port)
    : server_ip(ip)
    , server_port(port)
    , socket_initialized(false)
    , client_fd(-1)
{
}

Led_Client::~Led_Client()
{
    close_socket();
}

void Led_Client::create_socket()
{
    int opt;

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) 
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to open socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // enable SO_REUSEADDR to allow client socket rebind after closing without waiting for TIME_WAIT
    opt = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to set socket options: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    dbg_verbose("created socket %d", client_fd);
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

    if (client_fd < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Client bind received invalid socket descriptor: " << client_fd;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // connect to server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
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

void Led_Client::send_socket()
{
    led_msg_t client_msg;
    led_msg_t server_msg;
    ssize_t bytes_sent;
    ssize_t bytes_rcvd;
    const char *client_msg_data = "Message from client";

    // do not send to invalid socket
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send - socket is not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // create client message
    client_msg.magic = LED_MSG_MAGIC;
    client_msg.id = 0;
    client_msg.length = strlen(client_msg_data);
    memcpy(client_msg.data, client_msg_data, strlen(client_msg_data)+1);

    // send client message to server
    if ((bytes_sent = send(client_fd, &client_msg, sizeof(client_msg), 0)) != sizeof(client_msg))
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send: sent " << bytes_sent << " (expected " << sizeof(client_msg) << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Receive response from server
    if ((bytes_rcvd = recv(client_fd, &server_msg, sizeof(server_msg), 0)) != sizeof(server_msg))
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
}

void Led_Client::close_socket()
{
    if (client_fd >= 0)
    {
        dbg_notice("Close socket %d", client_fd);

        close(client_fd);
        client_fd = -1;
    }
}


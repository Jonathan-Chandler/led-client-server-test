#include "debug.h"
#include "led_server.h"
#include "led_frame.h"
#include <inttypes.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <errno.h>

Led_Server::Led_Server(int port)
    : server_port(port)
    , socket_initialized(false)
    , server_fd(-1)
{
}

Led_Server::~Led_Server()
{
    close_socket();
}

void Led_Server::create_socket()
{
    int opt;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) 
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to open socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // enable SO_REUSEADDR to allow server socket rebind after closing without waiting for TIME_WAIT
    opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to set socket options: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    dbg_notice("created socket %d", server_fd);
}

void Led_Server::bind_socket()
{
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (server_fd < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Server bind received invalid socket descriptor: " << server_fd;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to bind socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Update socket init status
    socket_initialized = true;

    dbg_notice("Led_Server successfully bind socket");
}

void Led_Server::listen_socket()
{
    led_msg_t client_msg;
    led_msg_t server_msg;
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // do not listen on invalid socket
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to listen - socket is not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    if (listen(server_fd, 5) < 0) 
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to listen on socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    while (1) 
    {
        dbg_notice("Led_Server listening for clients");

        // Accept a client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            dbg_error("Led_Server failed to create client fd");
            continue;
        }

        dbg_notice("Led_Server client connected");

        // Receive data from client
        recv(client_fd, &client_msg, sizeof(client_msg), 0);

        // check magic value and send response
        if (client_msg.magic == LED_MSG_MAGIC)
        {
            const char *response_msg = "Message from server";

            dbg_notice("Led_Server receive valid data from client: %s", client_msg.data);

            // set server response
            server_msg.magic = LED_MSG_MAGIC;
            server_msg.id = 0;
            server_msg.length = strlen(response_msg);
            memcpy(server_msg.data, response_msg, strlen(response_msg)+1);

            // Send response to client
            send(client_fd, &server_msg, sizeof(server_msg), 0);
        }
        else
        {
            dbg_error("Led_Server receive invalid data from client");
        }

        // Close client socket
        close(client_fd);

        dbg_notice("Led_Server client disconnect");
    }
}

void Led_Server::close_socket()
{
    if (server_fd >= 0)
    {
        dbg_notice("Close socket %d", server_fd);

        close(server_fd);
        server_fd = -1;
    }
}


#include <inttypes.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <chrono>
#include <thread>

#include "debug.h"
#include "led_server.h"


Led_Server::Led_Server(int port)
    : Led_Network()
    , server_port(port)
    , socket_initialized(false)
    , server_is_running(false)
    , valid_message_count(0)
{
}

Led_Server::~Led_Server()
{
    close_socket();
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

void Led_Server::start_server()
{
    led_msg_t client_msg;
    led_msg_t server_msg;
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    dbg_notice("starting server");

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

    // socket should not block so start_server can exit when given interrupt from stop_server() on another thread
    make_socket_nonblocking();

    // signal server is waiting for connections
    server_is_running.store(true);

    while (server_is_running.load()) 
    {
        dbg_notice("accepting clients");

        // accept client connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) 
        {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK)) 
            {
                // log if the error wasn't caused because no clients were connected
                dbg_error("Led_Server failed to handle client connection");
            }

            // sleep to avoid busy waiting for clients
            std::this_thread::sleep_for(std::chrono::milliseconds(LED_MESSAGE_POLL_TIME_MS));
            continue;
        }

        // record the connection
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

            // increment the number of valid client messages received
            valid_message_count++;
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

void Led_Server::stop_server()
{
    server_is_running.store(false);
}

bool Led_Server::get_server_is_running()
{
    return server_is_running.load();
}

void Led_Server::close_socket()
{
    dbg_notice("closing socket");

    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }
    else
    {
        dbg_error("attempted to close invalid socket %d", server_fd);
    }
}

void Led_Server::make_socket_nonblocking() 
{
    int flags;
    dbg_notice("setting socket to nonblocking mode");

    // get current socket flags
    if ( (flags = fcntl(server_fd, F_GETFL, 0)) == -1) 
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to get flags to make nonblocking socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // add option nonblocking to current flags
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1) 
    {
        std::ostringstream err_str;

        err_str << "Led_Server failed to set flags to make nonblocking socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }
}

int Led_Server::get_valid_message_count()
{
    return valid_message_count.load();
}


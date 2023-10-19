#include <inttypes.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <chrono>
#include <thread>

#include "debug.h"
#include "led_server.h"


Led_Server::Led_Server(int port)
    : Led_Network()
    , server_port(port)
    , server_is_running(false)
{
}

Led_Server::~Led_Server()
{
}

void Led_Server::bind_socket()
{
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    if (socket_fd < 0)
    {
        std::ostringstream err_str;

        err_str << "Led_Server bind received invalid socket descriptor: " << socket_fd;
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // Bind socket
    if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
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

#if 0
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

    if (listen(socket_fd, 5) < 0) 
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
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
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
#endif

void Led_Server::stop_server()
{
    server_is_running.store(false);
}

bool Led_Server::get_server_is_running()
{
    return server_is_running.load();
}

void Led_Server::start_server()
{
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

    if (listen(socket_fd, 5) < 0) 
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

    dbg_notice("accepting clients");
    while (server_is_running.load()) 
    {
        // accept client connection
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
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
        dbg_notice("client connected to server");

        // Receive data from client
        std::vector<uint8_t> client_frame = receive_all(client_fd);
        dbg_notice("received frame from client");
        dbg_verbose_print_vector(client_frame);

        Led_Strip client_leds(1);
        client_leds.set_leds_from_net_frame(client_frame);

        printf("converted configuration client: \n");
        client_leds.print_all_leds();

        // increment the number of valid messages received
        inc_receive_message_count();

        // Send response to client
        send_all(client_fd, client_frame);

        // increment the number of valid messages received
        inc_send_message_count();

        // Close client socket
        close(client_fd);

        dbg_notice("Led_Server client disconnect");
    }
}


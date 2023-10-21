#include <inttypes.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <errno.h>
#include <chrono>
#include <thread>
#include <future>

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

void Led_Server::initialize()
{
    dbg_notice("Initialize server");

    if (!socket_initialized)
    {
        // create TCP socket
        create_socket();

        // accept all IPv4 connections on server port
        bind_socket();

        // make server socket nonblocking to allow stop_server while polling for messages
        make_socket_nonblocking(socket_fd);

        // Update socket init status
        socket_initialized = true;
    }
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


    dbg_notice("Led_Server successfully bind socket");
}
void Led_Server::stop_server()
{
    server_is_running.store(false);
    stop_network();
}

bool Led_Server::get_server_is_running()
{
    return (server_is_running.load() && !stop_requested.load());
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

    // signal server is waiting for connections
    start_network();
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

        // do not block for client socket
        make_socket_nonblocking(client_fd);

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

Led_Server_Nonblocking::Led_Server_Nonblocking(int port)
    : server(port)
{
}

Led_Server_Nonblocking::~Led_Server_Nonblocking()
{
}

void Led_Server_Nonblocking::initialize()
{
    if (!socket_initialized)
    {
        server.initialize();

        socket_initialized = true;
    }
}

bool Led_Server_Nonblocking::get_server_is_running()
{
    return server.get_server_is_running();
}

int Led_Server_Nonblocking::get_send_message_count()
{
    return server.get_send_message_count();
}

int Led_Server_Nonblocking::get_receive_message_count()
{
    return server.get_receive_message_count();
}

void Led_Server_Nonblocking::start_server()
{
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // starting server that has already been started
    if (!server_thread_handle.valid())
    {
        server_thread_handle = std::async(std::launch::async, &Led_Server::start_server, &server);
    }
    else
    {
        std::ostringstream err_str;

        err_str << "Server is already running";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }
}

void Led_Server_Nonblocking::stop_server()
{
    // forward the stop request
    server.stop_server();

    // wait for 2 seconds before giving up and throwing an error
    if (server_thread_handle.wait_for(std::chrono::seconds(2)) != std::future_status::ready) 
    {
        std::ostringstream err_str;

        err_str << "Timed out while trying to stop server thread";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    try 
    {
        server_thread_handle.get();
    } 
    catch (const std::exception& e) 
    {
        std::ostringstream err_str;

        err_str << "server thread returned error: " << e.what();
        dbg_error("%s", err_str.str().c_str());
        throw;
    }
}


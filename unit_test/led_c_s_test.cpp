#include <iostream>
#include <future>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>

#include "unit_test.h"
#include "led.h"
#include "led_client.h"
#include "led_server.h"
#include "catch.hpp"

// LED client/server test
#define LOCAL_TEST_IP "127.0.0.1"
#define LOCAL_TEST_PORT 1632

void server_runner(Led_Server *test_server);
std::future<void> start_test_server(Led_Server &test_server);
void stop_test_server(Led_Server &test_server, std::future<void> &server_thread);

TEST_CASE("LED Client can be initialized", "[Led_Client::constructor]")
{
    Led_Client test_client = Led_Client(LOCAL_TEST_IP, LOCAL_TEST_PORT);

    try
    {
        test_client.create_socket();
        test_client.bind_socket();
        //test_client.send_socket();
    }
    catch (const std::runtime_error& runtime_err)
    {
        // expect runtime error because server is not running
        std::cerr << "Caught exception of an expected type: " << runtime_err.what() << std::endl;
        test_client.close_socket();
        REQUIRE(TEST_PASSES);
        return;
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        test_client.close_socket();
        REQUIRE(TEST_FAILS);
    }

    // expect runtime error
    REQUIRE(TEST_FAILS);
}

TEST_CASE("LED server can be initialized", "[Led_Server::constructor]")
{
    Led_Server test_server(LOCAL_TEST_PORT);
    
    try
    {
        test_server.create_socket();
        test_server.bind_socket();
    }
    catch (...)
    {
        // unknown error
        std::cerr << "unexpected exception while starting test_server" << std::endl;
        test_server.close_socket();
        REQUIRE(TEST_FAILS);
    }

    // run the server in its own thread in order to accept test client connection
    std::future<void> server_thread = std::async(std::launch::async, server_runner, &test_server);

    // wait maximum of 5 seconds for the server thread to start
    for (int i = 0; i < 10; i++)
    {
        // stop waiting when the server starts accepting connections
        if (test_server.get_server_is_running())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // request server thread to stop
    test_server.stop_server();

    // wait for a maximum of 2 seconds for the server to exit so tests will not hang if server fails
    auto status = server_thread.wait_for(std::chrono::seconds(2));
    if (status != std::future_status::ready) 
    {
        std::cerr << "Led_Server timed out after stop was requested" << std::endl;
        REQUIRE(TEST_FAILS);
    } 

    // server thread join
    try 
    {
        server_thread.get();
    } 
    catch (...)
    {
        std::cerr << "Led_Server thread returned unexpected exception" << std::endl;
        REQUIRE(TEST_FAILS);
    }
    
    REQUIRE(TEST_PASSES);
}

TEST_CASE("Led_Client can connect to Led_Server", "[Led_Client::send_socket]")
{
    Led_Client test_client(LOCAL_TEST_IP, LOCAL_TEST_PORT);
    Led_Server test_server(LOCAL_TEST_PORT);
    std::future<void> server_thread;
    
    // initialize test server and start waiting for client
    try
    {
        server_thread = start_test_server(test_server);
    }
    catch (...)
    {
        std::cerr << "Unexpected error while starting test server" << std::endl;
        REQUIRE(TEST_FAILS);
    }

    // attempt to connect/send to the server with test_client
    try
    {
        test_client.create_socket();
        test_client.bind_socket();
        test_client.send_socket();
    }
    catch (const std::runtime_error& runtime_err)
    {
        std::cerr << "Unexpected runtime error: " << runtime_err.what() << std::endl;
        test_client.close_socket();
        REQUIRE(TEST_FAILS);
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Unexpected error" << std::endl;
        test_client.close_socket();
        REQUIRE(TEST_FAILS);
    }

    // stop the test server
    try
    {
        stop_test_server(test_server, server_thread);
    }
    catch (...)
    {
        std::cerr << "Unexpected error while stopping test server" << std::endl;
        REQUIRE(TEST_FAILS);
    }

    // make sure server received the client message
    REQUIRE(test_server.get_valid_message_count() == 1);
}

// function to run server on separate thread so server_stop can be called from main thread
void server_runner(Led_Server *test_server)
{
    test_server->start_server();
}

std::future<void> start_test_server(Led_Server &test_server)
{
    // initialize the server socket
    test_server.create_socket();
    test_server.bind_socket();

    // wait up to 5 seconds for server to start on a new thread
    std::future<void> server_thread = std::async(std::launch::async, server_runner, &test_server);
    for (int i = 0; i < 10; i++)
    {
        // stop waiting when the server starts accepting connections
        if (test_server.get_server_is_running())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // throw an error if the server never started
    if (!test_server.get_server_is_running())
    {
        std::cerr << "fail to start test_server after 5 seconds" << std::endl;
        throw std::runtime_error("fail to start test_server after 5 seconds");
    }

    // return the thread with the running server
    return server_thread;
}

void stop_test_server(Led_Server &test_server, std::future<void> &server_thread)
{
    // request server stop
    test_server.stop_server();

    // wait 2 seconds for server to stop
    auto status = server_thread.wait_for(std::chrono::seconds(2));
    if (status != std::future_status::ready) 
    {
        std::cerr << "fail to stop test_server after 2 seconds" << std::endl;
        throw std::runtime_error("fail to stop test_server after 2 seconds");
    } 

    // forward any errors thrown by the server thread
    server_thread.get();
}
    

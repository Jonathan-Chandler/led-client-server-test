#include <iostream>
#include <future>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>

#include "led.h"
#include "led_client.h"
#include "led_server.h"
#include "catch.hpp"

// LED client/server test
#define LOCAL_TEST_IP "127.0.0.1"
#define LOCAL_TEST_PORT 1632

#if 0
void start_server_thread()
{
    Led_Server test_server = Led_Server(LOCAL_TEST_PORT);

    try
    {
        test_server.create_socket();
        test_server.bind_socket();
        test_server.listen_socket();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        test_server.close_socket();
        REQUIRE(1 == 0);
    }
}

void start_test_thread()
{
    printf("test\n");
    //throw std::runtime_error("An error has occurred!");
}
#endif

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
        // expected invalid argument error
        std::cerr << "Caught exception of an expected type: " << runtime_err.what() << std::endl;
        test_client.close_socket();
        REQUIRE(0 == 0);
        return;
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        test_client.close_socket();
        REQUIRE(1 == 0);
    }
}

#if 0
TEST_CASE("LED server can be initialized", "[Led_Server::constructor]")
{
    auto server_thread = std::async(std::launch::async, start_test_thread);
    
    std::cerr << "wait" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cerr << "done" << std::endl;

    try 
    {
        server_thread.get(); // Will rethrow the exception thrown in foo
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Caught exception: " << e.what() << "\n";
        REQUIRE(0 == 0);
        return;
    }
    
    std::cout << "This line will be executed even if an exception is thrown in the thread, because it is caught.\n";
    REQUIRE(1 == 0);
}
#endif


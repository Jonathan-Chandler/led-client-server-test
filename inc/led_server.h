#ifndef __LED_SERVER_H__
#define __LED_SERVER_H__
#include "led.h"
#include <atomic>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

#define LED_SERVER_POLL_TIME_MS 100     // server will wait 100 milliseconds between each poll for clients

class Led_Server
{
public:
    Led_Server(int port);
    ~Led_Server();

    void create_socket();
    void bind_socket();
    void start_server();
    void stop_server();
    bool get_server_is_running();
    void close_socket();
    int get_valid_message_count();

    //void send(std::string ipv4_addr, std::unique_ptr<Led_Strip::ser_led_strip_t> serialized_leds);
    //void receive();

private:
    struct sockaddr_in server_addr;
    int server_port;
    bool socket_initialized;
    int server_fd;
    std::atomic<bool> server_is_running;
    std::atomic<int> valid_message_count;

    void make_socket_nonblocking();
};

//int led_write_file(led_config_t *config, const char *file_name);
//int led_read_file(led_config_t **ret_config, const char *file_name);
//int led_append_file(led_config_t *config, FILE *file_ptr);
//int led_read_file_pointer(led_config_t **ret_config, FILE *file_ptr);

#endif // ifndef __Led_Server_H__


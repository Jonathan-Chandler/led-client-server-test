#ifndef __LED_SERVER_H__
#define __LED_SERVER_H__
#include "led.h"
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#ifndef DEBUG_PRINT
#define DEBUG_PRINT 1
#endif

class Led_Server
{
public:
    Led_Server(int port);
    ~Led_Server();

    void create_socket();
    void bind_socket();
    void listen_socket();
    void close_socket();

    //void send(std::string ipv4_addr, std::unique_ptr<Led_Strip::ser_led_strip_t> serialized_leds);
    //void receive();

private:
    struct sockaddr_in server_addr;
    int server_port;
    bool socket_initialized;
    int server_fd;
};

//int led_write_file(led_config_t *config, const char *file_name);
//int led_read_file(led_config_t **ret_config, const char *file_name);
//int led_append_file(led_config_t *config, FILE *file_ptr);
//int led_read_file_pointer(led_config_t **ret_config, FILE *file_ptr);

#endif // ifndef __Led_Server_H__


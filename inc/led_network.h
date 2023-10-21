#ifndef __LED_NETWORK_H__
#define __LED_NETWORK_H__

#include <chrono>
#include <vector>
#include <atomic>

#define LED_MESSAGE_POLL_TIME_MS    100                                         // wait 100 milliseconds between each failed poll for received messages
#define LED_MESSAGE_TIMEOUT_MS      3000                                        // wait 3 seconds to receive messages before giving up / triggering receive failure

class Led_Network
{
public:
    Led_Network();
    virtual ~Led_Network();

    int get_send_message_count();
    int get_receive_message_count();
    void inc_send_message_count();
    void inc_receive_message_count();
    void start_network();
    void stop_network();

protected:
    int socket_fd;
    bool socket_initialized;
    std::atomic<int> send_message_count;
    std::atomic<int> receive_message_count;
    std::atomic<bool> stop_requested;

    void create_socket();
    void close_socket();
    void make_socket_nonblocking(int config_socket);
    bool check_tcp_timeout(const std::chrono::time_point<std::chrono::high_resolution_clock>& start);
    void send_all(int dst_socket, const std::vector<uint8_t> &led_frame);
    std::vector<uint8_t> receive_all(int src_socket);

};

#endif // __LED_NETWORK_H__

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
    void create_socket();
    void close_socket();
    int get_send_message_count();
    int get_receive_message_count();
    void inc_send_message_count();
    void inc_receive_message_count();

protected:
    int socket_fd;
    bool socket_initialized;
    std::atomic<int> send_message_count;
    std::atomic<int> receive_message_count;

    void make_socket_nonblocking();
    bool tcp_receive_timeout(const std::chrono::time_point<std::chrono::high_resolution_clock>& start);
    void send_all(int dest_socket, const std::vector<uint8_t> &led_frame);
    std::vector<uint8_t> receive_all(int src_socket);

};

#endif // __LED_NETWORK_H__

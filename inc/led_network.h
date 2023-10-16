#ifndef __LED_NETWORK_H__
#define __LED_NETWORK_H__

#include <chrono>
#include <vector>

#define LED_MESSAGE_POLL_TIME_MS    100                                         // wait 100 milliseconds between each failed poll for received messages
#define LED_MESSAGE_TIMEOUT_MS      3000                                        // wait 3 seconds to receive messages before giving up / triggering receive failure

class Led_Network
{
public:
    Led_Network();
    virtual ~Led_Network();
    void create_socket();
    void close_socket();

protected:
    int sock_fd;
    bool tcp_receive_timeout(const std::chrono::time_point<std::chrono::high_resolution_clock>& start);
    void send_all(const std::vector<uint8_t> &buffer);
    std::vector<uint8_t> receive_all();

};

#endif // __LED_NETWORK_H__

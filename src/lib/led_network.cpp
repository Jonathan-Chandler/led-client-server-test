#include <sstream>
#include <stdexcept>
#include <thread>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "led.h"
#include "led_network.h"
#include "debug.h"


Led_Network::Led_Network()
    : socket_fd(-1)
    , socket_initialized(false)
{
}

Led_Network::~Led_Network()
{
    close_socket();
}

void Led_Network::create_socket()
{
    int opt;

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    {
        std::ostringstream err_str;

        err_str << "failed to open socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // enable SO_REUSEADDR to allow client socket rebind after closing without waiting for TIME_WAIT
    opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::ostringstream err_str;

        err_str << "failed to set socket options: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    dbg_verbose("created socket %d", socket_fd);
}

void Led_Network::make_socket_nonblocking() 
{
    int flags;
    dbg_notice("setting socket to nonblocking mode");

    // get current socket flags
    if ( (flags = fcntl(socket_fd, F_GETFL, 0)) == -1) 
    {
        std::ostringstream err_str;

        err_str << "failed to get flags to make nonblocking socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // add option nonblocking to current flags
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) 
    {
        std::ostringstream err_str;

        err_str << "failed to set flags to make nonblocking socket: " << strerror(errno) << " (" << errno << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }
}

void Led_Network::close_socket()
{
    if (socket_fd >= 0)
    {
        dbg_notice("close socket %d", socket_fd);

        close(socket_fd);
        socket_fd = -1;
    }
}

bool Led_Network::tcp_receive_timeout(
            const std::chrono::time_point<std::chrono::high_resolution_clock>& start
        )
{
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    return (elapsed >= std::chrono::milliseconds(LED_MESSAGE_TIMEOUT_MS));
}


void Led_Network::send_all(const std::vector<uint8_t> &led_frame)
{
    ssize_t bytes_sent = 0;
    ssize_t total_bytes_sent = 0;
    const uint8_t *send_ptr;
    ssize_t expected_size = led_frame.size();
    ssize_t remaining_size;

    // do not send to invalid socket
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send - socket is not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // don't allow messages bigger than server buffer
    if (led_frame.size() > LED_BUFFER_MAX_SIZE)
    {
        std::ostringstream err_str;

        err_str << "led_frame is too long - receive " << led_frame.size() << " (maximum " << LED_BUFFER_MAX_SIZE << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
        
    }

    // send client message to server
    while (total_bytes_sent < led_frame.size())
    {
        // update current buffer pointer / remaining number of bytes to be sent
        send_ptr = &led_frame.data()[total_bytes_sent];
        remaining_size = expected_size - total_bytes_sent;

        // attempt to send to server
        bytes_sent = send(socket_fd, send_ptr, remaining_size, 0);
        if (bytes_sent == -1)
        {
            std::ostringstream err_str;

            err_str << "Led_Client failed to send: sent " << total_bytes_sent << " of expected total bytes " << expected_size;
            dbg_error("%s", err_str.str().c_str());
            throw std::runtime_error(err_str.str());
        }

        // update bytes successfully sent
        total_bytes_sent += bytes_sent;
    }
}

std::vector<uint8_t> Led_Network::receive_all()
{
    ssize_t bytes_recv = 0;
    ssize_t total_bytes_recv = 0;
    uint8_t *recv_ptr;
    ssize_t expected_size;
    ssize_t remaining_size;
    std::vector<uint8_t> led_header(LED_HEADER_SIZE);
    std::vector<uint8_t> led_frame;
    Led_Strip::led_net_t *header_data;
    uint32_t led_count;
    auto start_time = std::chrono::high_resolution_clock::now();

    // do not send to invalid socket
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send - socket is not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // wait for the header
    expected_size = LED_HEADER_SIZE;
    while (total_bytes_recv < LED_HEADER_SIZE)
    {
        // timed out while waiting for message
        if (tcp_receive_timeout(start_time))
        {
            // failed while getting data
            std::ostringstream err_str;

            err_str << "Timed out while waiting for message: recv " << total_bytes_recv << " of expected total bytes " << expected_size;
            dbg_error("%s", err_str.str().c_str());
            throw std::runtime_error(err_str.str());
        }

        // update current buffer pointer / remaining number of bytes to be sent
        recv_ptr = &led_header.data()[total_bytes_recv];
        remaining_size = expected_size - total_bytes_recv;

        // attempt to send to server
        bytes_recv = recv(socket_fd, recv_ptr, remaining_size, 0);
        if (bytes_recv == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                // no data is waiting - wait 
                std::this_thread::sleep_for(std::chrono::milliseconds(LED_MESSAGE_POLL_TIME_MS));

                continue;
            } 
            else 
            {
                // failed while getting data
                std::ostringstream err_str;

                err_str << "Led_Client failed to get header: recv " << total_bytes_recv << " of expected total bytes " << expected_size;
                dbg_error("%s", err_str.str().c_str());
                throw std::runtime_error(err_str.str());
            }
        }

        // update bytes successfully sent
        total_bytes_recv += bytes_recv;
    }

    // check header was rcev
    if (total_bytes_recv != LED_HEADER_SIZE)
    {
        std::ostringstream err_str;

        err_str << "couldn't get led header - receive " << total_bytes_recv << " (expect " << LED_HEADER_SIZE << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // check header is valid and get led count
    header_data = reinterpret_cast<Led_Strip::led_net_t*>(led_header.data());
    led_count = ntohl(header_data->net_led_count);
    if (led_count < 1 || led_count > LED_MAX_COUNT)
    {
        std::ostringstream err_str;

        err_str << "led_count was invalid - receive " << led_count << " (range 1-" << LED_MAX_COUNT << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // create the final buffer with space to store LED header and data
    expected_size = LED_HEADER_SIZE + (led_count * sizeof(Led_Strip::led_color_t));
    led_frame.resize(expected_size);
    if (led_frame.size() != expected_size)
    {
        std::string err = "Failed to allocate space for led frame";
        dbg_error("%s", err.c_str());
        throw std::runtime_error(err);
    }

    // copy header to final led_frame
    memcpy(led_frame.data(), led_header.data(), LED_HEADER_SIZE);

    // get led color data (expected_size and total_bytes_recv include sizeof header+colors
    while (total_bytes_recv < expected_size)
    {
        // update current buffer pointer / remaining number of bytes to be sent
        recv_ptr = &led_frame.data()[total_bytes_recv];
        remaining_size = expected_size - total_bytes_recv;

        // timed out while waiting for message
        if (tcp_receive_timeout(start_time))
        {
            std::ostringstream err_str;

            err_str << "Timed out while waiting for message: recv " << total_bytes_recv << " of expected total bytes " << expected_size;
            dbg_error("%s", err_str.str().c_str());
            throw std::runtime_error(err_str.str());
        }

        // attempt to send to server
        bytes_recv = recv(socket_fd, recv_ptr, remaining_size, 0);
        if (bytes_recv == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // no data waiting
                continue;
            } 
            else 
            {
                // failed while getting data
                std::ostringstream err_str;

                err_str << "Led_Client failed to get header: recv " << total_bytes_recv << " of expected total bytes " << expected_size;
                dbg_error("%s", err_str.str().c_str());
                throw std::runtime_error(err_str.str());
            }
        }

        // update bytes successfully sent
        total_bytes_recv += bytes_recv;
    }

    return led_frame;
}

#if 0
void Led_Client::send_leds(std::vector<uint8_t> led_frame)
{
    ssize_t total_bytes_sent = 0;
    ssize_t bytes_sent = 0;
    ssize_t bytes_rcvd = 0;
    std::vector<uint8_t> receive_data;

    // do not send to invalid socket
    if (!socket_initialized)
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to send - socket is not initialized";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    // don't allow messages bigger than server buffer
    if (led_frame.size() > LED_BUFFER_SIZE)
    {
        std::ostringstream err_str;

        err_str << "led_frame is too long - receive " << led_frame.size() << " (maximum " << LED_BUFFER_SIZE << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
        
    }

    // send client message to server
    while (total_bytes_sent < led_frame.size())
    {
        bytes_sent = send(socket_fd, &client_msg, sizeof(client_msg), 0);
        if (bytes_sent == -1)
        {
            std::ostringstream err_str;

            err_str << "Led_Client failed to send: sent " << bytes_sent << " (expected " << sizeof(client_msg) << ")";
            dbg_error("%s", err_str.str().c_str());
            throw std::runtime_error(err_str.str());
        }
    }

    // Receive response from server
    if ((bytes_rcvd = recv(socket_fd, &server_msg, sizeof(server_msg), 0)) != sizeof(server_msg))
    {
        std::ostringstream err_str;

        err_str << "Led_Client failed to receive server message: receive " << bytes_rcvd << " (expected " << sizeof(server_msg) << ")";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }

    if (server_msg.magic == LED_MSG_MAGIC)
    {
        std::ostringstream notice_str;
        notice_str << "Led_Client received data: " << server_msg.data;
        dbg_notice("%s", notice_str.str().c_str());
    }
    else
    {
        std::ostringstream err_str;
        err_str << "Led_Client received invalid magic value from server";
        dbg_error("%s", err_str.str().c_str());
        throw std::runtime_error(err_str.str());
    }
}
#endif



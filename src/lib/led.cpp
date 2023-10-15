#include <memory>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <arpa/inet.h>

#include "debug.h"
#include "share.h"
#include "led.h"

// loaded LED file must start with this string
const std::string Led_Strip::led_magic = std::string(LED_MAGIC);
const Led_Strip::led_color_t Led_Strip::led_color_white = {255, 255, 255};
const int Led_Strip::led_file_min_len = (led_magic.length() + sizeof(led_color_t));
const int Led_Strip::led_file_max_len = (Led_Strip::led_file_min_len + (WS2812_LED_COUNT*sizeof(led_color_t)));

Led_Strip::Led_Strip(int led_count_arg)
    : led_strip(led_count_arg, led_color_white)
{
}

Led_Strip::Led_Strip(int led_count_arg, const led_color_t *led_color)
{
    if (led_color == nullptr)
    {
        std::string err_str = "Led_Strip initialized with null led_color";
        dbg_error(err_str.c_str());
        throw std::invalid_argument(err_str);
    }

    led_strip = std::vector<led_color_t>(led_count_arg, *led_color);
}

Led_Strip::Led_Strip(int led_count_arg, uint8_t red_val, uint8_t green_val, uint8_t blue_val)
    : led_strip(led_count_arg, {.red=red_val, .green=green_val, .blue=blue_val})
{
}

Led_Strip::Led_Strip(const char *file_path)
    : led_strip(1, led_color_white)
{
    load_all_leds(file_path);
}

Led_Strip::~Led_Strip()
{
}

int Led_Strip::get_led_count()
{
    return led_strip.size();
}

std::unique_ptr<Led_Strip::led_color_t> Led_Strip::get_led_color(uint32_t led_index)
{
    std::unique_ptr<led_color_t> return_led = std::unique_ptr<led_color_t>(new led_color_t);;

    if (led_index >= led_strip.size())
    {
        std::ostringstream err_str;

        err_str << "get_led_color index " << led_index << " higher than maximum index " << led_strip.size()-1;
        throw std::invalid_argument(err_str.str());
    }

    // unique_ptr<led_color_t> failed to allocate
    if (return_led == nullptr)
        throw std::runtime_error("Failed to allocate LED color");

    // set values
    return_led->red = led_strip[led_index].red;
    return_led->green = led_strip[led_index].green;
    return_led->blue = led_strip[led_index].blue;

    return return_led;
}

Led_Strip& Led_Strip::set_led_color(uint32_t led_index, const led_color_t *led_color)
{
    if (led_color == nullptr)
    {
        throw std::invalid_argument("Led_Strip initialized with null led_color");
    }

    return set_led_color(led_index, led_color->red, led_color->green, led_color->blue);
}

Led_Strip& Led_Strip::set_led_color(uint32_t led_index, uint8_t red_value, uint8_t green_value, uint8_t blue_value)
{
    if (led_index >= led_strip.size())
    {
        std::ostringstream err_str;

        err_str << "Led_Strip::set_led_color index " << led_index << " higher than maximum index " << led_strip.size()-1;
        throw std::invalid_argument(err_str.str());
    }

    led_strip[led_index].red = red_value;
    led_strip[led_index].green = green_value;
    led_strip[led_index].blue = blue_value;

    return *this;
}

Led_Strip& Led_Strip::set_all_leds(const led_color_t *led_color)
{
    if (led_color == nullptr)
    {
        throw std::invalid_argument("set_all_leds received null led_color");
    }

    // reset all color values
    std::fill(led_strip.begin(), led_strip.end(), *led_color);

    return *this;
}

Led_Strip& Led_Strip::set_all_leds(uint8_t red_value, uint8_t green_value, uint8_t blue_value)
{
    led_color_t led_color = {.red=red_value, .green=green_value, .blue=blue_value};

    return set_all_leds(&led_color);
}

Led_Strip& Led_Strip::set_led_color_range(uint32_t start_index, uint32_t end_index, const led_color_t *led_color)
{
    if (start_index > end_index)
    {
        std::ostringstream err_str;

        err_str << "set_led_color_range start index " << start_index << " higher than ending index " << end_index;
        throw std::invalid_argument(err_str.str());
    }

    if (start_index >= led_strip.size())
    {
        std::ostringstream err_str;

        err_str << "set_led_color_range start index " << start_index << " higher than maximum index " << led_strip.size()-1;
        throw std::invalid_argument(err_str.str());
    }

    if (end_index >= led_strip.size())
    {
        std::ostringstream err_str;

        err_str << "set_led_color_range end index " << start_index << " higher than maximum index " << led_strip.size()-1;
        throw std::invalid_argument(err_str.str());
    }

    if (led_color == nullptr)
    {
        throw std::invalid_argument("set_led_color_range received null led_color");
    }

    for (uint32_t i = start_index; i <= end_index; i++)
    {
        led_strip[i] = *led_color;
    }

    return *this;
}

Led_Strip& Led_Strip::set_led_color_range(uint32_t start_index, uint32_t end_index, uint8_t red_value, uint8_t green_value, uint8_t blue_value)
{
    led_color_t led_color = {.red=red_value, .green=green_value, .blue=blue_value};

    return set_led_color_range(start_index, end_index, &led_color);
}

Led_Strip& Led_Strip::print_led(uint32_t led_index)
{
    if (led_index >= led_strip.size())
    {
        std::ostringstream err_str;

        err_str << "print_led index " << led_index << " higher than maximum index " << led_strip.size()-1;
        throw std::invalid_argument(err_str.str());
    }

    std::cout << "{" << led_strip[led_index].red << ", " << led_strip[led_index].green << ", " << led_strip[led_index].blue << "}" << std::endl;

    return *this;
}

Led_Strip& Led_Strip::print_all_leds()
{
    std::cout << "---------------------------------" << std::endl;
    std::cout << "         Print " << (int)led_strip.size() << " LEDs" << std::endl;

    for (uint32_t i = 0; i < led_strip.size(); i++)
    {
        std::cout 
            << "[" 
            << std::setw(3)
            << std::setfill(' ')
            << i 
            << "] {" 
            << std::hex 
            << std::uppercase 
            << std::setw(2)
            << std::setfill('0')
            << (int)led_strip[i].red 
            << ", " 
            << std::setw(2)
            << std::setfill('0')
            << (int)led_strip[i].green 
            << ", " 
            << std::setw(2)
            << std::setfill('0')
            << (int)led_strip[i].blue 
            << "}" << std::endl
            << std::dec;
    }
    std::cout << "---------------------------------" << std::endl;

    return *this;
}

Led_Strip& Led_Strip::load_all_leds(const char *file_path)
{
    std::streampos file_size = 0;
    char *buffer = nullptr;
    std::ostringstream err_str;
    std::ifstream input_file(file_path, std::ios::binary);

    if (!input_file)
    {
        throw std::runtime_error("Led_Strip data file could not be read");
    }

    // get file length
    file_size = input_file.tellg();
    input_file.seekg( 0, std::ios::end );
    file_size = input_file.tellg() - file_size;

    std::cout << "file_size = " << file_size << std::endl;

    // reject if file length not in range
    if (file_size < led_file_min_len || file_size > led_file_max_len)
    {
        err_str << "Led_Strip data file not in range (" << led_file_min_len << " - " << led_file_max_len << ")";
        throw std::runtime_error(err_str.str());
    }

    // create buffer and reset file pointer
    buffer = new char [file_size];
    if (!buffer)
    {
        err_str << "Led_Strip failed to allocate file buffer";
        throw std::runtime_error(err_str.str());
    }

    // read to buffer and close input file
    input_file.seekg(0, std::ios::beg);
    input_file.read(buffer,file_size);
    input_file.close();

    // check for magic value
    for (int i = 0; i < led_magic.length(); i++)
    {
        if (buffer[i] != led_magic[i])
        {
            delete[] buffer;

            err_str << "Led_Strip data file was not valid - did not start with 'LEDS'";
            throw std::runtime_error(err_str.str());
        }
    }

    // check if remaining file is evenly divisible by sizeof(led_color_t)
    if (((size_t)file_size - led_magic.length()) % sizeof(led_color_t))
    {
        delete[] buffer;

        err_str << "Led_Strip data file was not valid - contains invalid LED definition (" 
            << ((size_t)file_size - led_magic.length())  
            << " % "
            << sizeof(led_color_t)
            << " = " 
            << (((size_t)file_size - led_magic.length()) % sizeof(led_color_t))
            << " (expected 0)";
        throw std::runtime_error(err_str.str());
    }

    // erase current leds
    led_strip.clear();

    // read led values
    int x;
    for (x = led_magic.length(); x < file_size; x+=sizeof(led_color_t))
    {
        if ((x + sizeof(led_color_t)) <= file_size)
        {
            led_color_t *current_led = (led_color_t*) &buffer[x];
            led_strip.push_back(*current_led);
        }
        else
        {
            delete[] buffer;
            err_str << "Led_Strip data file was not valid - bad file len";
            throw std::runtime_error(err_str.str());
        }
    }
    x -= led_magic.length();
    x /= sizeof(led_color_t);

    std::cout << "read " << x << " led definitions" << std::endl;
    delete[] buffer;
    return *this;
}

Led_Strip& Led_Strip::save_all_leds(const char *file_path)
{
    const char *led_magic_buffer = led_magic.c_str();
    size_t led_magic_buffer_size = led_magic.length();
    char *led_buffer = (char*) &led_strip[0];
    size_t led_buffer_size = led_strip.size()*sizeof(led_color_t);
    std::ofstream output_file(file_path, std::ios::trunc | std::ios::binary);

    if (!output_file.is_open())
    {
        std::ostringstream err_str;
        err_str << "Led_Strip save leds failed to open file " << file_path;
        throw std::runtime_error(err_str.str());
    }

    // write magic and led data
    output_file.write(led_magic_buffer, led_magic_buffer_size);
    output_file.write(led_buffer, led_buffer_size);

    // close ofstream
    output_file.close();

    return *this;
}

int Led_Strip::get_led_net_frame_size()
{
    // total bytes required to store led frame
    // returns: 4 + 4 + 3*led_count
    return (LED_MAGIC_LEN + sizeof(uint32_t) + (get_led_count() * sizeof(led_color_t)));
}

std::vector<uint8_t> Led_Strip::get_led_net_frame()
{
    std::vector<uint8_t> led_frame_data(get_led_net_frame_size());
    uint32_t net_led_count = htonl(get_led_count());
    char magic_str[] = LED_MAGIC;
    uint8_t *frame_ptr;

    if (led_frame_data.size() < get_led_net_frame_size())
    {
        std::string err = "Failed to allocate space for frame";
        dbg_error("%s", err.c_str());
        throw std::runtime_error(err);
    }

    // copy magic string
    frame_ptr = led_frame_data.data();
    memcpy(frame_ptr, magic_str, LED_MAGIC_LEN);

    // copy led count
    frame_ptr += LED_MAGIC_LEN;
    memcpy(frame_ptr, &net_led_count, sizeof(net_led_count));

    // copy led rgb values
    frame_ptr += sizeof(net_led_count);
    memcpy(frame_ptr, led_strip.data(), led_strip.size());

    return led_frame_data;
}

Led_Strip& Led_Strip::set_leds_serialized(std::unique_ptr<char[]> &led_data)
{
    return *this;
}

#if 0
led_net_t* create_led_net(size_t num_leds) 
{
    led_net_t* p = (led_net_t*)malloc(sizeof(led_net_t) + num_leds * sizeof(led_color_t));

    p->net_led_count = num_leds;

    return p;
}
#endif


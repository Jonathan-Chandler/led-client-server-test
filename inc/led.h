#ifndef __LED_H__
#define __LED_H__
#include <memory>
#include <stdint.h>
#include <vector>

#define LED_MAGIC       "LEDS"
#define LED_MAGIC_LEN   (sizeof(LED_MAGIC) - 1)

class Led_Strip
{
public:
    typedef struct led_color_t
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } __attribute__((packed)) led_color_t;

    // format used to send led data over network
    typedef struct led_net_t
    {
        char led_magic[LED_MAGIC_LEN];
        uint32_t net_led_count;
        led_color_t raw_led_data[0];
    }  __attribute__((packed)) led_net_t;

    // initialize led strip with led_count leds with all color values 255
    Led_Strip(int led_count);

    Led_Strip(int led_count, const led_color_t *led_color);

    // initialize strip with all leds as a single color
    Led_Strip(int led_count, uint8_t red_val, uint8_t green_val, uint8_t blue_val);

    // initialize from file
    Led_Strip(const char *file_path);

    ~Led_Strip();

    //led_color_t *get_led_color(uint32_t led_index);
    int get_led_count();
    std::unique_ptr<led_color_t> get_led_color(uint32_t led_index);

    Led_Strip& set_led_color(uint32_t led_index, const led_color_t *led_color);
    Led_Strip& set_led_color(uint32_t led_index, uint8_t red_value, uint8_t green_value, uint8_t blue_value);

    Led_Strip& set_all_leds(const led_color_t *led_color);
    Led_Strip& set_all_leds(uint8_t red_value, uint8_t green_value, uint8_t blue_value);

    Led_Strip& set_led_color_range(uint32_t start_index, uint32_t end_index, const led_color_t *led_color);
    Led_Strip& set_led_color_range(uint32_t start_index, uint32_t end_index, uint8_t red_value, uint8_t green_value, uint8_t blue_value);

    Led_Strip& print_led(uint32_t led_index);
    Led_Strip& print_all_leds();

    Led_Strip& load_all_leds(const char *file_path);
    Led_Strip& save_all_leds(const char *file_path);

    int get_led_net_frame_size();
    std::vector<uint8_t> get_led_net_frame();
    Led_Strip& set_leds_from_net_frame(std::vector<uint8_t> &net_frame);

private:
    std::vector<led_color_t> led_strip;
    static const led_color_t led_color_white;
    static const std::string led_magic;
    static const int led_file_min_len;
    static const int led_file_max_len;
};

//int led_write_file(led_config_t *config, const char *file_name);
//int led_read_file(led_config_t **ret_config, const char *file_name);
//int led_append_file(led_config_t *config, FILE *file_ptr);
//int led_read_file_pointer(led_config_t **ret_config, FILE *file_ptr);

#endif // __LED_H__

//class Led_Strip
//{
//public:
//    typedef struct led_net_t
//    {
//        char led_magic[led_magic.length()];
//        uint8_t *data;
//    }  __attribute__((packed)) led_net_t;
//
//private:
//    static const std::string led_magic;
//};
//

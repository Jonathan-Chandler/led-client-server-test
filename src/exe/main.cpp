#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <unistd.h>
#include "debug.h"
#include "led.h"
#include "share.h"

#define MAX_FILE_NAME_LEN 256
#define TEST_CYCLE_TIME_2_SEC 2000 // in milliseconds

char filename[MAX_FILE_NAME_LEN];
debug_mode_t debug_mode = DEBUG_ERROR;

uint8_t led_count = 0;
uint8_t red_value = 0;
uint8_t green_value = 0;
uint8_t blue_value = 0;

void usage(const char *executable_name);
int parse_args(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    std::unique_ptr<Led_Strip> leds;

    if (parse_args(argc, argv) < 0)
        return -1;

    if (led_count || red_value || green_value || blue_value)
    {
        if (!led_count)
        {
            fprintf(stderr, "LED manual configuration requires option LED count (-c)\n");
            return -1;
        }
        dbg_notice("Initialize %" PRIu8 " LEDs to RGB values {0x%02" PRIx8 ",0x%02" PRIx8 ",0x%02" PRIx8 "}", 
                led_count, red_value, green_value, blue_value);

        // initialize with manual configuration
        try
        {
            leds = std::unique_ptr<Led_Strip>(new Led_Strip(led_count, red_value, green_value, blue_value));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            return -1;
        }

        leds->save_all_leds("./saved_leds.dat");
        leds->print_all_leds();
        return 0;
    }
    else if (filename[0] != 0)
    {
        // initialize by loading from file
        try
        {
            leds = std::unique_ptr<Led_Strip>(new Led_Strip(filename));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
            return -1;
        }

        leds->print_all_leds();
        return 0;
    }
    else
    {
        // require arg
        usage(argv[0]);
    }

    return -1;
}

void usage(const char *executable_name)
{
    fprintf(stderr, "usage: %s [-d] [[-c led_count] [-r value] [-g value] [-b value] OR [-l input_file]]\n", executable_name);
    fprintf(stderr, "        -h               - print this help text\n");
    fprintf(stderr, "        -d <mode>        - set debug logging mode (0-%d)\n", (DEBUG_MODE_COUNT-1));
    fprintf(stderr, "\n");
    fprintf(stderr, "    Configure LEDs manually\n");
    fprintf(stderr, "        -c <led_count>   - number of LEDs connected\n");
    fprintf(stderr, "        -r <red_value>   - LED red value (0-255)\n");
    fprintf(stderr, "        -g <green_value> - LED green value (0-255)\n");
    fprintf(stderr, "        -b <blue_value>  - LED blue value (0-255)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    Load LED configuration from file\n");
    fprintf(stderr, "        -l <filename>    - file with led format\n");
    fprintf(stderr, "\n");
}

int parse_args(int argc, char *argv[])
{
    int opt; 
    const char *short_opt = "hd:c:r:g:b:l:";
    struct option long_opt[] =
    {
        {"help",          no_argument,       NULL, 'h'},
        {"debug",         required_argument, NULL, 'd'},
        {"count",         required_argument, NULL, 'c'},
        {"red",           required_argument, NULL, 'r'},
        {"green",         required_argument, NULL, 'g'},
        {"blue",          required_argument, NULL, 'b'},
        {"load",          required_argument, NULL, 'l'},
        {NULL,            0,                 NULL, 0  }
    };

    while (1)
    {
        opt = getopt_long(argc, argv, short_opt, long_opt, NULL);

        // end of options
        if (opt == -1)
            break;

        switch (opt)
        {
            // debug mode
            case 'd':
                if (isdigit(optarg[0]) && (optarg[1] >= 0 && optarg[1] < DEBUG_MODE_COUNT))
                {
                    debug_mode = (debug_mode_t) atoi(optarg);
                }
                else
                {
                    fprintf(stderr, "Argument for -d must be in range 0-%d\n", (DEBUG_MODE_COUNT-1));
                    return -1;
                }
                dbg_verbose("set debug mode: %d", debug_mode);
                break;

            // led count
            case 'c':
                uint32_t leds_value;

                if (isdigit(optarg[0]))
                {
                    leds_value = (uint32_t) atoi(optarg);
                }
                else
                {
                    fprintf(stderr, "Argument for -c must be an integer\n");
                    return -1;
                }
                if ((leds_value < 1) || (leds_value > WS2812_LED_COUNT))
                {
                    fprintf(stderr, "Argument for -%c must be an integer in range 1-%d\n", opt, WS2812_LED_COUNT);
                    return -1;
                }

                led_count = leds_value & 0xFF;
                dbg_verbose("set led count: %" PRIu8 "", led_count);

                // attempting set led count while using load file
                if (filename[0] != 0)
                {
                    fprintf(stderr, "Option -%c can't be used with -l <filename>\n", opt);
                    return -1;
                }
                break;

            // led red/green/blue value
            case 'r':
            case 'g':
            case 'b':
                uint32_t color_value;

                if (isdigit(optarg[0]))
                {
                    color_value = (uint32_t) atoi(optarg);
                }
                else
                {
                    fprintf(stderr, "Argument for -%c must be an integer in range 0-255\n", opt);
                    return -1;
                }
                if (color_value > 255)
                {
                    fprintf(stderr, "Argument for -%c must be an integer in range 0-255\n", opt);
                    return -1;
                }
                switch (opt)
                {
                    case 'r':
                        red_value = color_value & 0xFF;
                        dbg_verbose("set red value: %" PRIu8 "", red_value);
                        break;
                    case 'g':
                        green_value = color_value & 0xFF;
                        dbg_verbose("set green value: %" PRIu8 "", green_value);
                        break;
                    case 'b':
                        blue_value = color_value & 0xFF;
                        dbg_verbose("set blue value: %" PRIu8 "", blue_value);
                        break;
                    default:
                        fprintf(stderr, "Unknown color option %c\n", opt);
                        return -1;
                }

                // attempting to load file with colors set
                if (filename[0] != 0)
                {
                    fprintf(stderr, "Option -%c can't be used with -l <filename>\n", opt);
                    return -1;
                }
                break;

            // load file
            case 'l':
                strncpy(filename , optarg, sizeof(filename));
                dbg_verbose("set file name: %s", filename);

                // attempting to load file with colors set
                if (led_count != 0)
                {
                    fprintf(stderr, "Option -%c can't be used with manual LED count\n", opt);
                    return -1;
                }
                if (red_value != 0 || green_value != 0 || blue_value != 0)
                {
                    fprintf(stderr, "Option -%c can't be used with manual LED color configuration\n", opt);
                    return -1;
                }
                break;

            case 'h':
            default:
                usage(argv[0]);
                return -1;
        }
    }

    return 0;
}
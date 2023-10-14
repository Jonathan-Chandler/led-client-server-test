//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER 1
#include "debug.h"
#include "catch.hpp"
#include <iostream>

debug_mode_t debug_mode = DEBUG_VERBOSE;

int main(int argc, char* argv[])
{
    int return_code;
    int failed_tests;
    Catch::Session session;

    if ( (return_code = session.applyCommandLine(argc, argv)) != 0)
        return return_code;

    failed_tests = session.run();

    return failed_tests;
}

#if 0
    // initialize as white
    leds->print_all_leds();
    delete leds;

    // initialize as green
    try
    {
        leds = new Led_Strip(12, 0, 255, 0);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
    leds->print_all_leds();
    delete leds;

    // initialize as blue
    try
    {
        Led_Strip::led_color_t led_color = {.red = 0, .green = 0, .blue = 255};
        leds = new Led_Strip(12, &led_color);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
    leds->print_all_leds();

    // set 3-5 to red
    try
    {
        Led_Strip::led_color_t led_color = {.red = 255, .green = 0, .blue = 0};
        leds->set_led_color_range(3, 5, &led_color);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
    leds->print_all_leds();

    // set led 8 to green
    try
    {
        Led_Strip::led_color_t led_color = {.red = 0, .green = 255, .blue = 0};
        leds->set_led_color_range(8, 8, &led_color);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }

    // set led 11 to green/red
    try
    {
        Led_Strip::led_color_t led_color = {.red = 255, .green = 255, .blue = 0};
        leds->set_led_color(11, &led_color);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }
    leds->print_all_leds();
    delete leds;

    std::cout << "exit" << std::endl;
    return 0;
}
#endif

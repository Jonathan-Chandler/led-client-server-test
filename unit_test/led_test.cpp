#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include "led.h"
#include "catch.hpp"

TEST_CASE("LEDs are initialized to 255 for all values", "[LedStrip::constructor]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t test_led;

    // expected init values
    test_led.red = 255;
    test_led.green = 255;
    test_led.blue = 255;

    leds = new Led_Strip(12);
    REQUIRE(leds != nullptr);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);
        compare = memcmp(led.get(), &test_led, sizeof(test_led));
        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("LEDs are initialized to expected values", "[LedStrip::constructor]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t test_led = {.red = 0, .green = 255, .blue = 0};

    // expected init values
    test_led.red = 0;
    test_led.green = 255;
    test_led.blue = 0;

    // initialize as green
    leds = new Led_Strip(12, 0, 255, 0);
    REQUIRE(leds != nullptr);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);
        compare = memcmp(led.get(), &test_led, sizeof(test_led));
        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("LEDs are initialized to expected values using integers", "[LedStrip::constructor]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t test_led = {.red = 0, .green = 255, .blue = 0};

    // expected init values
    test_led.red = 0;
    test_led.green = 255;
    test_led.blue = 0;

    // initialize as green
    leds = new Led_Strip(12, 0, 255, 0);
    REQUIRE(leds != nullptr);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);
        compare = memcmp(led.get(), &test_led, sizeof(test_led));
        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("LEDs are initialized to expected values using led_color_t", "[LedStrip::constructor]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t test_led = {.red = 0, .green = 0, .blue = 255};

    // initialize as green
    leds = new Led_Strip(12, &test_led);
    REQUIRE(leds != nullptr);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);
        compare = memcmp(led.get(), &test_led, sizeof(test_led));
        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("LEDs can be set by index range", "[LedStrip::set_led_color_range]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t new_color = {.red = 0, .green = 0, .blue = 255};
    Led_Strip::led_color_t starting_color = {.red = 255, .green = 255, .blue = 255};

    // initialize as green
    leds = new Led_Strip(12, &starting_color);
    REQUIRE(leds != nullptr);

    // change led colors at index 3-5
    leds->set_led_color_range(3, 5, &new_color);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);

        if (i >= 3 && i <=5)
        {
            compare = memcmp(led.get(), &new_color, sizeof(new_color));
        }
        else
        {
            compare = memcmp(led.get(), &starting_color, sizeof(new_color));
        }
        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("Single LED can be set by index range", "[LedStrip::set_led_color_range]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t new_color = {.red = 255, .green = 0, .blue = 0};
    Led_Strip::led_color_t starting_color = {.red = 255, .green = 255, .blue = 255};

    // initialize as green
    leds = new Led_Strip(12, &starting_color);
    REQUIRE(leds != nullptr);

    // change led color at index 8
    leds->set_led_color_range(8, 8, &new_color);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);

        if (i == 8)
        {
            compare = memcmp(led.get(), &new_color, sizeof(new_color));
        }
        else
        {
            compare = memcmp(led.get(), &starting_color, sizeof(new_color));
        }

        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("Single LED can be set by index", "[LedStrip::set_led_color]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t new_color = {.red = 255, .green = 255, .blue = 0};
    Led_Strip::led_color_t starting_color = {.red = 255, .green = 255, .blue = 255};

    // initialize as green
    leds = new Led_Strip(12, &starting_color);
    REQUIRE(leds != nullptr);

    // change led color at index 8
    leds->set_led_color(11, &new_color);

    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led = leds->get_led_color(i);

        if (i == 11)
        {
            compare = memcmp(led.get(), &new_color, sizeof(new_color));
        }
        else
        {
            compare = memcmp(led.get(), &starting_color, sizeof(new_color));
        }

        REQUIRE(compare == 0);
    }

    delete(leds);
}

TEST_CASE("set_led_color fails if index is out of range", "[LedStrip::set_led_color]")
{
    Led_Strip *leds;
    Led_Strip::led_color_t starting_color = {.red = 255, .green = 255, .blue = 0};

    // initialize as green
    leds = new Led_Strip(12, &starting_color);
    REQUIRE(leds != nullptr);

    // change led color at index 8
    try
    {
        leds->set_led_color(12, &starting_color);
    }
    catch (const std::invalid_argument& ia)
    {
        // expected invalid argument error
        std::cerr << "Caught exception of an expected type: " << ia.what() << std::endl;
        REQUIRE(0 == 0);
        delete(leds);
        leds = nullptr;
        return;
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        REQUIRE(1 == 0);
    }

    // should not happen
    REQUIRE(1 == 0);
}

TEST_CASE("save_all_leds and load_all_leds return expected values", "[LedStrip::save_load_all_leds]")
{
    Led_Strip *saved_leds = nullptr;
    Led_Strip *loaded_leds = nullptr;
    Led_Strip::led_color_t starting_color = {.red = 255, .green = 255, .blue = 0};
    Led_Strip::led_color_t another_color = {.red = 0, .green = 0, .blue = 255};
    const char *file_path = "./test_leds.dat";
    int led_count = 15;

    // initialize as green
    saved_leds = new Led_Strip(led_count, &starting_color);
    REQUIRE(saved_leds != nullptr);

    // change led color at index 8
    try
    {
        saved_leds->set_led_color(12, &another_color);
        saved_leds->save_all_leds(file_path);
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        REQUIRE(1 == 0);
    }

    try
    {
        loaded_leds = new Led_Strip(file_path);
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        REQUIRE(1 == 0);
    }

    // check number of leds
    REQUIRE(saved_leds->get_led_count() == led_count);
    REQUIRE(saved_leds->get_led_count() == loaded_leds->get_led_count());

    // check leds equal
    for (int i = 0; i < saved_leds->get_led_count(); i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> saved_color = saved_leds->get_led_color(i);
        std::unique_ptr<Led_Strip::led_color_t> loaded_color = loaded_leds->get_led_color(i);

        REQUIRE(saved_color->red == loaded_color->red);
        REQUIRE(saved_color->green == loaded_color->green);
        REQUIRE(saved_color->blue == loaded_color->blue);
    }

    if (loaded_leds != nullptr)
        delete(loaded_leds);

    if (saved_leds != nullptr)
        delete(saved_leds);
}


#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>

#include "unit_test.h"
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
        REQUIRE(TEST_PASSES);
        delete(leds);
        leds = nullptr;
        return;
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        REQUIRE(TEST_FAILS);
    }

    // should not happen
    REQUIRE(TEST_FAILS);
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
        REQUIRE(TEST_FAILS);
    }

    try
    {
        loaded_leds = new Led_Strip(file_path);
    }
    catch (...)
    {
        // unknown error
        std::cerr << "Caught an exception of an unexpected type." << std::endl;
        REQUIRE(TEST_FAILS);
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

TEST_CASE("get_led_net_frame returns valid vector", "[LedStrip::get_led_net_frame]")
{
    Led_Strip *leds;
    int compare = 0;
    Led_Strip::led_color_t test_led = {.red = 0, .green = 255, .blue = 0};
    char expected_magic[] = LED_MAGIC;
    int led_count;

    // expected init values
    test_led.red = 0;
    test_led.green = 255;
    test_led.blue = 0;

    // initialize as green
    leds = new Led_Strip(12, 0, 255, 0);
    REQUIRE(leds != nullptr);

    std::vector<uint8_t> net_frame = leds->get_led_net_frame();

    // 44 = 4+4+12*3 -> strlen('LEDS') + sizeof(uint32_t) + led_count*sizeof(led_color_t)
    int expected_size = strlen("LEDS") + sizeof(uint32_t) + 12*sizeof(Led_Strip::led_color_t);
    printf("expected size = %d\n", expected_size);

    // size matches
    REQUIRE(net_frame.size() == expected_size);

    // reinterpret to check matches led_net_t layout
    Led_Strip::led_net_t *converted_data = reinterpret_cast<Led_Strip::led_net_t*>(net_frame.data());

    // magic string matches
    for (int i = 0; i < LED_MAGIC_LEN; i++)
    {
        REQUIRE(converted_data->led_magic[i] == expected_magic[i]);
    }

    // size value in net frame matches (12 leds)
    led_count = ntohl(converted_data->net_led_count);
    REQUIRE(led_count == 12);

    // check if led count matches given the number bytes remaining in net_frame
    int remaining_bytes = net_frame.size() - sizeof(Led_Strip::led_net_t);
    int expected_remaining_bytes = led_count * sizeof(Led_Strip::led_color_t);

    REQUIRE(remaining_bytes == expected_remaining_bytes);

#if 0
    // data values match
    for (int i = 8; i < expected_remaining_bytes; i++)
    {
        printf("&led[%d] = %p\n", i, &net_frame.data()[i]);
        printf("led[%d] = %X\n", i, net_frame.data()[i]);
        printf("\n");
    }
#endif

    // led values match
    for (int i = 0; i < led_count; i++)
    {
        REQUIRE(memcmp(&converted_data->raw_led_data[i], &test_led, sizeof(Led_Strip::led_color_t)) == 0);
    }

    delete(leds);
}

TEST_CASE("get_led_net_frame returns matches set_led_net_frame", "[LedStrip::set_led_net_frame]")
{
    Led_Strip leds(12, 0, 255, 0);
    Led_Strip leds_copy(19);
    int compare = 0;
    char expected_magic[] = LED_MAGIC;

    std::vector<uint8_t> net_frame = leds.get_led_net_frame();
    leds_copy.set_leds_from_net_frame(net_frame);

    // led count matches copied leds
    REQUIRE(leds.get_led_count() == leds_copy.get_led_count());
    REQUIRE(leds.get_led_count() == 12);

    // leds match input values
    for (int i = 0; i < 12; i++)
    {
        std::unique_ptr<Led_Strip::led_color_t> led_1 = leds.get_led_color(i);
        std::unique_ptr<Led_Strip::led_color_t> led_2 = leds_copy.get_led_color(i);
        REQUIRE(memcmp(led_1.get(), led_2.get(), sizeof(Led_Strip::led_color_t)) == 0);
    }
}


#ifndef __LED_MESSAGE_H__
#define __LED_MESSAGE_H__

#include <stdint.h>

#define LED_MSG_MAGIC 0x4C454453

typedef struct 
{
    uint32_t magic;
    uint32_t id;
    uint16_t length;
    char data[256];
} led_msg_t;

#endif

#ifndef GAME_IPC_H
#define GAME_IPC_H

#include <stdint.h>
#include "cmsis_os.h"

typedef struct
{
    int8_t dx;
    uint8_t btn;
    uint16_t raw;
} InputMsg_t;

#ifdef __cplusplus
extern "C" {
#endif

extern osMessageQueueId_t joyQueueHandle;

#ifdef __cplusplus
}
#endif

#endif

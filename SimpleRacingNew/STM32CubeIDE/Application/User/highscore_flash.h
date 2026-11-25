#ifndef HIGHSCORE_FLASH_H
#define HIGHSCORE_FLASH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t HighScore_Load(void);
void HighScore_Save(uint32_t score);

#ifdef __cplusplus
}
#endif

#endif

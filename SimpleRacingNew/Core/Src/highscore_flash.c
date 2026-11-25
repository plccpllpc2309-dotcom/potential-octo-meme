#include "highscore_flash.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#define HS_FLASH_ADDR   0x081E0000U
#define HS_FLASH_SECTOR FLASH_SECTOR_23
#define HS_MAGIC_WORD   0x52414345U

typedef struct
{
    uint32_t magic;
    uint32_t score;
    uint32_t scoreInv;
} HighScoreRecord_t;

uint32_t HighScore_Load(void)
{
    const volatile HighScoreRecord_t *rec = (const volatile HighScoreRecord_t *)HS_FLASH_ADDR;

    if ((rec->magic == HS_MAGIC_WORD) && (rec->scoreInv == ~rec->score))
    {
        return rec->score;
    }
    return 0U;
}

void HighScore_Save(uint32_t score)
{
    if (HighScore_Load() == score)
    {
        return;
    }

    FLASH_EraseInitTypeDef eraseInit = {0};
    uint32_t sectorError = 0U;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = HS_FLASH_SECTOR;
    eraseInit.NbSectors = 1U;

    vTaskSuspendAll();
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) == HAL_OK)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HS_FLASH_ADDR, HS_MAGIC_WORD) == HAL_OK)
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HS_FLASH_ADDR + 4U, score) == HAL_OK)
            {
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HS_FLASH_ADDR + 8U, ~score);
            }
        }
    }

    HAL_FLASH_Lock();
    xTaskResumeAll();
}

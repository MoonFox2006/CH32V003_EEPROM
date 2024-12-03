#pragma once

#include <stdint.h>
#include <ch32v00x_flash.h>

#define EEPROM_INPLACE

#define EEPROM_OBSIZE   8
#define EEPROM_SIZE     (32 - EEPROM_OBSIZE)

#ifdef EEPROM_INPLACE
uint8_t EEPROM_Init(const void *addr, uint8_t size);
void EEPROM_Refresh(void);
FLASH_Status EEPROM_Flush(void);
FLASH_Status EEPROM_Clear(void);
#else
void EEPROM_Init(void);
void EEPROM_Refresh(void);
FLASH_Status EEPROM_Flush(void);
FLASH_Status EEPROM_Clear(void);

uint8_t EEPROM_Read(uint8_t index);
void EEPROM_Write(uint8_t index, uint8_t value);
#endif

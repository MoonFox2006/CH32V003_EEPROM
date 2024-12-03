#include <ch32v00x.h>
#include "eeprom.h"

#define EEPROM_ADDR (OB_BASE + EEPROM_OBSIZE * 2)

static FLASH_Status FLASH_WriteOptionBytes(uint32_t address, const uint8_t *data, uint8_t size) {
    FLASH_Status status = FLASH_COMPLETE;

    while (size--) {
        *(__IO uint16_t*)address = *data++;
        status = FLASH_WaitForLastOperation(0x2000);
        if (status != FLASH_COMPLETE)
            break;
        address += 2;
    }
    return status;
}

static FLASH_Status FLASH_ProgramOptionBytes(const uint8_t *data, uint8_t size) {
    FLASH_Status status;

    FLASH_Unlock();
    status = FLASH_WaitForLastOperation(0x000B0000);
    if (status == FLASH_COMPLETE) {
        uint8_t ob[6];

        ob[0] = OB->RDPR;
        ob[1] = OB->USER;
        ob[2] = OB->Data0;
        ob[3] = OB->Data1;
        ob[4] = OB->WRPR0;
        ob[5] = OB->WRPR1;

        FLASH->OBKEYR = FLASH_MODEKEYR_KEY1;
        FLASH->OBKEYR = FLASH_MODEKEYR_KEY2;

        FLASH->CTLR |= (uint32_t)FLASH_CTLR_OPTER;
        FLASH->CTLR |= (uint32_t)FLASH_CTLR_STRT;
        status = FLASH_WaitForLastOperation(0xB0000);
        if (status == FLASH_COMPLETE) {
            FLASH->CTLR &= ~((uint32_t)FLASH_CTLR_OPTER);
            FLASH->CTLR |= (uint32_t)FLASH_CTLR_OPTPG;

            status = FLASH_WriteOptionBytes(OB_BASE, ob, sizeof(ob));
            if (status == FLASH_COMPLETE) {
                if (data && size) {
                    status = FLASH_WriteOptionBytes(EEPROM_ADDR, data, size);
                }
            }
        }
        if (status != FLASH_TIMEOUT) {
            FLASH->CTLR &= ~((uint32_t)FLASH_CTLR_OPTPG);
        }
    }
    FLASH_Lock();
    return status;
}

#ifdef EEPROM_INPLACE
static uint16_t _addr;
static uint8_t _size = 0;

uint8_t EEPROM_Init(const void *addr, uint8_t size) {
    if ((size > 0) && (size <= EEPROM_SIZE) && (((uint32_t)addr & 0xFFFF0000) == SRAM_BASE)) {
        _addr = (uint32_t)addr & 0xFFFF;
        _size = size;
        EEPROM_Refresh();
        return 1;
    }
    return 0;
}

void EEPROM_Refresh(void) {
    for (uint8_t i = 0; i < _size; ++i) {
        *(uint8_t*)((SRAM_BASE | _addr) + i) = *(__IO uint8_t*)(EEPROM_ADDR + (i << 1));
    }
}

inline __attribute__((always_inline)) FLASH_Status EEPROM_Flush(void) {
    return FLASH_ProgramOptionBytes((uint8_t*)(SRAM_BASE | _addr), _size);
}

inline __attribute__((always_inline)) FLASH_Status EEPROM_Clear(void) {
    return FLASH_ProgramOptionBytes((uint8_t*)0, 0);
}

#else
static uint8_t _eeprom[EEPROM_SIZE];

inline __attribute__((always_inline)) void EEPROM_Init(void) {
    EEPROM_Refresh();
}

void EEPROM_Refresh(void) {
    for (uint8_t i = 0; i < EEPROM_SIZE; ++i) {
        _eeprom[i] = *(__IO uint8_t*)(EEPROM_ADDR + (i << 1));
    }
}

inline __attribute__((always_inline)) FLASH_Status EEPROM_Flush(void) {
    return FLASH_ProgramOptionBytes(_eeprom, EEPROM_SIZE);
}

inline __attribute__((always_inline)) FLASH_Status EEPROM_Clear(void) {
    return FLASH_ProgramOptionBytes((uint8_t*)0, 0);
}

inline __attribute__((always_inline)) uint8_t EEPROM_Read(uint8_t index) {
    return _eeprom[index];
}

inline __attribute__((always_inline)) void EEPROM_Write(uint8_t index, uint8_t value) {
    _eeprom[index] = value;
}
#endif

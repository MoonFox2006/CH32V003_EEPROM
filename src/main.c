#include "debug.h"
#include "eeprom.h"

#define LED_RCC     RCC_APB2Periph_GPIOC
#define LED_GPIO    GPIOC
#define LED_PIN     GPIO_Pin_0

struct __attribute__((__packed__)) config_t {
    uint32_t param32;
    uint16_t param16;
    uint8_t param8;
} config;

static void LED_Init(void) {
    const GPIO_InitTypeDef gpio_cfg = {
        .GPIO_Pin = LED_PIN,
        .GPIO_Speed = GPIO_Speed_2MHz,
        .GPIO_Mode = GPIO_Mode_Out_PP
    };

    RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);
    GPIO_Init(LED_GPIO, (GPIO_InitTypeDef*)&gpio_cfg);
}

static inline void LED_Toggle(void) {
    LED_GPIO->OUTDR ^= LED_PIN;
}

int main(void) {
    SystemCoreClockUpdate();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Delay_Init();
#if SDI_PRINT == SDI_PR_OPEN
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    LED_Init();

#ifdef EEPROM_INPLACE
    if (EEPROM_Init(&config, sizeof(config))) {
#else
    EEPROM_Init();
    config.param32 = EEPROM_Read(0) | (EEPROM_Read(1) << 8) | (EEPROM_Read(2) << 16) | (EEPROM_Read(3) << 24);
    config.param16 = EEPROM_Read(4) | (EEPROM_Read(5) << 8);
    config.param8 = EEPROM_Read(6);
#endif
        printf("\n\"EEPROM\" dump\n");
        printf("param32: %lu, param16: %u, param8: %u\n", config.param32, config.param16, config.param8);
        ++config.param32;
        ++config.param16;
        ++config.param8;
#ifndef EEPROM_INPLACE
    EEPROM_Write(0, config.param32 & 0xFF);
    EEPROM_Write(1, (config.param32 >> 8) & 0xFF);
    EEPROM_Write(2, (config.param32 >> 16) & 0xFF);
    EEPROM_Write(3, (config.param32 >> 24) & 0xFF);
    EEPROM_Write(4, config.param16 & 0xFF);
    EEPROM_Write(5, (config.param16 >> 8) & 0xFF);
    EEPROM_Write(6, config.param8);
#endif
        if (EEPROM_Flush() != FLASH_COMPLETE) {
            printf("\"EEPROM\" write error!\n");
        }
#ifdef EEPROM_INPLACE
    } else {
        printf("\"EEPROM\" init error!\n");
    }
#endif

    while (1) {
        LED_Toggle();
        Delay_Ms(1000);
        printf("param32: %lu, param16: %u, param8: %u\n", config.param32, config.param16, config.param8);
    }
}

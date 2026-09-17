#include "force_backend.h"
#include "board_config.h"
#include "stm32f4xx_hal.h"
static void Hx711_Delay(void)
{
    uint32_t t = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - t) < SystemCoreClock / 1000000U)
    {
    }
}
static ArmResult Hx711_Read(int32_t *raw)
{
    if (!raw)
        return ARM_INVALID_ARGUMENT;
    if (HAL_GPIO_ReadPin(BOARD_HX_PORT, BOARD_HX_DOUT_PIN) != GPIO_PIN_RESET)
        return ARM_NOT_READY;
    uint32_t bits = 0;
    /* 25 pulses, A/128. PRIMASK only while SCK is high; low phases are interruptible. */
    for (unsigned i = 0; i < 25; ++i)
    {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        BOARD_HX_PORT->BSRR = BOARD_HX_SCK_PIN;
        Hx711_Delay();
        if (i < 24)
            bits = (bits << 1) | ((BOARD_HX_PORT->IDR & BOARD_HX_DOUT_PIN) ? 1U : 0U);
        BOARD_HX_PORT->BSRR = (uint32_t)BOARD_HX_SCK_PIN << 16;
        __set_PRIMASK(primask);
        Hx711_Delay();
    }
    *raw = Hx711_SignExtend(bits);
    return ARM_OK;
}
const ForceBackend *ForceBackend_Get(void)
{
    static const ForceBackend b = {Hx711_Read};
    return &b;
}

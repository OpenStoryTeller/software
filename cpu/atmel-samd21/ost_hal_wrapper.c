#include "ost_common.h"

#include "spi_master.h"
#include "uart.h"

static volatile uint32_t msTicks = 0;

// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------
void system_putc(char ch)
{
    uart_putc(ch);
}

void irq_handler_sys_tick()
{
    msTicks++;
}

void system_delay_ms(uint32_t delay)
{
    uint32_t curTicks;
 
    curTicks = msTicks;
    while ((msTicks - curTicks) < delay) ;
}


// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void spi_initialize(uint8_t id)
{
    spi_init(100000, 0);
}

void spi_set_fclk_slow()
{
    spi_init(100000, 0);
}

void spi_set_fclk_fast()
{
    spi_init(800000, 0);
}

void spi_cs_high()
{
    spi_ss(1);
}

void spi_cs_low()
{
    spi_ss(0);
}

uint8_t xchg_spi (uint8_t dat)
{
    return spi_write_byte(dat);
}

void rcvr_spi_multi (uint8_t *buff, uint32_t btr)
{
    for (uint32_t i = 0; i < btr; i++)
    {
        buff[i] = xchg_spi(0xff);
    }
}



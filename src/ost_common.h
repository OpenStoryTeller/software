#ifndef OST_COMMON_H
#define OST_COMMON_H

#include <stdint.h>

#define OST_ID_SPI_FOR_SDCARD   0

#define portDISABLE_INTERRUPTS()	__asm volatile( "csrc mstatus, 8" )
#define portENABLE_INTERRUPTS()		__asm volatile( "csrs mstatus, 8" )

#define portNOP() __asm volatile 	( " nop " )


// ----------------------------------------------------------------------------
// SHARED TYPES
// ----------------------------------------------------------------------------
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} rect_t;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;


// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------
void system_initialize();
void system_putc(char ch);
void system_delay_ms(uint32_t delay);

// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void spi_set_fclk_slow();
void spi_set_fclk_fast();
void spi_cs_high();
void spi_cs_low();
void spi_initialize(uint8_t id);

/**
 * @brief
 *
 * @param dat Data to send
 * @return uint8_t
 */
uint8_t xchg_spi (uint8_t dat);

/**
 * @brief Receive multiple byte
 *
 * @param buff  Pointer to data buffer
 * @param btr	Number of bytes to receive (even number)
 */
void rcvr_spi_multi (uint8_t *buff, uint32_t btr);

// ----------------------------------------------------------------------------
// DISPLAY HAL
// ----------------------------------------------------------------------------
void disp_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette);


#endif // OST_COMMON_H

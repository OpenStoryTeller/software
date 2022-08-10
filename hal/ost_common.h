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
void sdcard_set_slow_clock();
void sdcard_set_fast_clock();
void sdcard_cs_high();
void sdcard_cs_low();

/**
 * @brief
 *
 * @param dat Data to send
 * @return uint8_t
 */
uint8_t sdcard_spi_transfer (uint8_t dat);

/**
 * @brief Receive multiple byte
 *
 * @param buff  Pointer to data buffer
 * @param btr	Number of bytes to receive (even number)
 */
void sdcard_spi_recv_multi (uint8_t *buff, uint32_t btr);

// ----------------------------------------------------------------------------
// DISPLAY HAL
// ----------------------------------------------------------------------------
void display_initialize();
void disp_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette);
void display_dc_high();
void display_dc_low();

#endif // OST_COMMON_H

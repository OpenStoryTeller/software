#include "ost_common.h"

#include "samd21.h"
#include "uart.h"
#include "hal_gpio.h"
#include "spi_master.h"
#include "uart.h"

#include "string.h"
#include "debug.h"

#include "arduino_platform.h"

static volatile uint32_t msTicks = 0;

//-----------------------------------------------------------------------------
#define PERIOD_FAST     1200
#define PERIOD_SLOW     500

HAL_GPIO_PIN(LED,      B,  8) // Built-in LED
HAL_GPIO_PIN(BUTTON,   A, 15)

HAL_GPIO_PIN(CD,              A, 27);

//-----------------------------------------------------------------------------
static void timer_set_period(uint16_t i)
{
  TC3->COUNT16.CC[0].reg = (F_CPU / 1000ul / 256) * i;
  TC3->COUNT16.COUNT.reg = 0;
}

//-----------------------------------------------------------------------------
void TC3_Handler(void)
{
  if (TC3->COUNT16.INTFLAG.reg & TC_INTFLAG_MC(1))
  {
    HAL_GPIO_LED_toggle();
    TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC(1);
  }
}

//-----------------------------------------------------------------------------
static void timer_init(void)
{
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(TC3_GCLK_ID) |
      GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

  TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ |
      TC_CTRLA_PRESCALER_DIV256 | TC_CTRLA_PRESCSYNC_RESYNC;

  TC3->COUNT16.COUNT.reg = 0;

  timer_set_period(PERIOD_SLOW);

  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;

  TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC(1);
  NVIC_EnableIRQ(TC3_IRQn);
}


void arduino_mkrzero_init( void );

// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------
void system_initialize()
{
    arduino_mkrzero_init();

    SysTick_Config(F_CPU / 1000);

    timer_init();
    uart_init(115200);


    HAL_GPIO_LED_out();
    HAL_GPIO_LED_clr();

    timer_set_period(PERIOD_FAST);

    // Initialize SPI
    spi_init(250000, 0);

    HAL_GPIO_CD_in(); 
    HAL_GPIO_CD_pullup();


  //   sd_card_test();

/*
  uart_puts("\r\nHello, world!\r\n");

  HAL_GPIO_LED_out();
  HAL_GPIO_LED_clr();

  HAL_GPIO_BUTTON_in();
  HAL_GPIO_BUTTON_pullup();

  while (1)
  {
    if (HAL_GPIO_BUTTON_read())
      cnt = 0;
    else if (cnt < 5001)
      cnt++;

    if (5000 == cnt)
    {
      fast = !fast;
      timer_set_period(fast ? PERIOD_FAST : PERIOD_SLOW);
      uart_putc('.');
    }
  }*/
}


void system_putc(char ch)
{
    uart_putc(ch);
}

void SysTick_Handler()
{
    msTicks++;
}

void system_delay_ms(uint32_t delay)
{
    uint32_t curTicks = msTicks;
    while ((msTicks - curTicks) < delay) ;
}


// ----------------------------------------------------------------------------
// SDCARD HAL
// ----------------------------------------------------------------------------
void sdcard_set_slow_clock()
{
    spi_init(100000, 0);
}

void sdcard_set_fast_clock()
{
    spi_init(800000, 0);
}

void sdcard_cs_high()
{
    spi_ss(1);
}

void sdcard_cs_low()
{
    spi_ss(0);
}

uint8_t sdcard_spi_transfer (uint8_t dat)
{
    return spi_transfer(dat);
}

void sdcard_spi_recv_multi (uint8_t *buff, uint32_t btr)
{
    for (uint32_t i = 0; i < btr; i++)
    {
        buff[i] = spi_transfer(0xff);
    }
}


// ----------------------------------------------------------------------------
// DISPLAY HAL
// ----------------------------------------------------------------------------
#define DC_PIN GPIO_PIN_1
#define DC_GPIO_PORT GPIOA
#define DC_GPIO_CLK RCU_GPIOA

void ost_display_initialize()
{

}

void ost_display_dc_high()
{

}

void ost_display_dc_low()
{

}

void ost_display_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{

}

uint8_t ost_display_transfer_byte(uint8_t dat)
{
    return spi_display_transfer(dat);
}

void ost_display_transfer_multi (uint8_t *buff, uint32_t btr)
{
    for (uint32_t i = 0; i < btr; i++)
    {
        buff[i] = spi_display_transfer(0xff);
    }
}

/*
 * Arduino Zero board initialization
 *
 * Good to know:
 *   - At reset, ResetHandler did the system clock configuration. Core is running at 48MHz.
 *   - Watchdog is disabled by default, unless someone plays with NVM User page
 *   - During reset, all PORT lines are configured as inputs with input buffers, output buffers and pull disabled.
 */
void arduino_mkrzero_init( void )
{
  // // Set Systick to 1ms interval, common to all Cortex-M variants
  // if ( SysTick_Config( SystemCoreClock / 1000 ) )
  // {
  //   // Capture error
  //   while ( 1 ) ;
  // }
  NVIC_SetPriority (SysTick_IRQn,  (1 << __NVIC_PRIO_BITS) - 2);  /* set Priority for Systick Interrupt (2nd lowest) */

  // Clock PORT for Digital I/O
  PM->APBBMASK.reg |= PM_APBBMASK_PORT ;
//
//  // Clock EIC for I/O interrupts
  PM->APBAMASK.reg |= PM_APBAMASK_EIC ;

  // Clock SERCOM for Serial
  PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0 | PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM5 ;

  // Clock TC/TCC for Pulse and Analog
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 ;

  // Clock ADC/DAC for Analog
  PM->APBCMASK.reg |= PM_APBCMASK_ADC | PM_APBCMASK_DAC ;

// Defining VERY_LOW_POWER breaks Arduino APIs since all pins are considered INPUT at startup
// However, it really lowers the power consumption by a factor of 20 in low power mode (0.03mA vs 0.6mA)
#ifdef VERY_LOW_POWER
  // Setup all pins (digital and analog) in INPUT mode (default is nothing)
  for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  {
    pinMode( ul, INPUT ) ;
  }
#endif

  // Initialize Analog Controller
  // Setting clock
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_ADC ) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN ;

  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 |    // Divide Clock by 512.
                   ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default

  ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length

  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;   // No Negative input (Internal Ground)

  // Averaging (see datasheet table in AVGCTRL register description)
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |    // 1 sample only (no oversampling nor averaging)
                     ADC_AVGCTRL_ADJRES(0x0ul);   // Adjusting result by 0

  // analogReference( AR_DEFAULT ) ; // Analog Reference is AREF pin (3.3v)

  // Initialize DAC
  // Setting clock
  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY );
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_DAC ) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN ;

  while ( DAC->STATUS.bit.SYNCBUSY == 1 ); // Wait for synchronization of registers between the clock domains
  DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | // Using the 3.3V reference
                   DAC_CTRLB_EOEN ;        // External Output Enable (Vout)

}


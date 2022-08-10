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



uint8_t const CMD0 = 0X00;
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
uint8_t const CMD8 = 0X08;
/** SEND_CSD - read the Card Specific Data (CSD register) */
uint8_t const CMD9 = 0X09;
/** SEND_CID - read the card identification information (CID register) */
uint8_t const CMD10 = 0X0A;
/** SEND_STATUS - read the card status register */
uint8_t const CMD13 = 0X0D;
/** READ_BLOCK - read a single data block from the card */
uint8_t const CMD17 = 0X11;
/** WRITE_BLOCK - write a single data block to the card */
uint8_t const CMD24 = 0X18;
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
uint8_t const CMD25 = 0X19;
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
uint8_t const CMD32 = 0X20;
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
uint8_t const CMD33 = 0X21;
/** ERASE - erase all previously selected blocks */
uint8_t const CMD38 = 0X26;
/** APP_CMD - escape for application specific command */
uint8_t const CMD55 = 0X37;
/** READ_OCR - read the OCR register of a card */
uint8_t const CMD58 = 0X3A;
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
uint8_t const ACMD23 = 0X17;
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
uint8_t const ACMD41 = 0X29;
//------------------------------------------------------------------------------
/** status for card in the ready state */
uint8_t const R1_READY_STATE = 0X00;
/** status for card in the idle state */
uint8_t const R1_IDLE_STATE = 0X01;
/** status bit for illegal command */
uint8_t const R1_ILLEGAL_COMMAND = 0X04;
/** start data token for read or write single block*/
uint8_t const DATA_START_BLOCK = 0XFE;
/** stop token for write multiple blocks*/
uint8_t const STOP_TRAN_TOKEN = 0XFD;
/** start data token for write multiple blocks*/
uint8_t const WRITE_MULTIPLE_TOKEN = 0XFC;
/** mask for data response tokens after a write block operation */
uint8_t const DATA_RES_MASK = 0X1F;
/** write data accepted token */
uint8_t const DATA_RES_ACCEPTED = 0X05;

uint8_t status_ = 0;

static void spiSend(uint8_t b) {
  spi_transfer(b);
}
/** Receive a byte from the card */
static  uint8_t spiRec(void) {
  return spi_transfer(0xFF);
}

uint32_t millis()
{
    return msTicks;
}

uint8_t waitNotBusy(unsigned int timeoutMillis) {
  unsigned int t0 = millis();
  unsigned int d;
  do {
    if (spiRec() == 0XFF) {
      return true;
    }
    d = millis() - t0;
  } while (d < timeoutMillis);
  return false;
}

void chipSelectLow()
{
    spi_ss(0);
}

uint8_t cardCommand(uint8_t cmd, uint32_t arg) {

  // select card
  chipSelectLow();

  // wait up to 300 ms if busy
  waitNotBusy(300);

  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) {
    spiSend(arg >> s);
  }

  // send CRC
  uint8_t crc = 0XFF;
  if (cmd == CMD0) {
    crc = 0X95;  // correct crc for CMD0 with arg 0
  }
  if (cmd == CMD8) {
    crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
  }
  spiSend(crc);

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++)
    ;
  return status_;
}

static void sd_card_test(void)
{
  bool inserted  = false;

  debug_printf("--- SD Card Test ---\n");

  inserted = !HAL_GPIO_CD_read();

  debug_printf("SD Card is %s\n", inserted ? "inserted" : "not inserted");

  if (inserted)
  {
      unsigned int t0 = millis();

      for (uint8_t i = 0; i < 10; i++) {
        spiSend(0XFF);
      }

      spi_ss(0);
      bool error = false;

      while ((status_ = cardCommand(CMD0, 0)) != 0x01) {
        unsigned int d = millis() - t0;
        if (d > 2000) {
            error = true;
            break;
        }
      }

      if  (error) {
        debug_printf("Failure :(\n");
      } else {
        debug_printf("SUCCESS !\n");
      }

      spi_ss(1);
  }
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

void display_initialize()
{

}

void display_dc_high()
{

}

void display_dc_low()
{

}

void disp_draw_h_line(uint16_t y, uint8_t *pixels, uint8_t *palette)
{

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


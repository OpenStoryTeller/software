#include "ost_common.h"

#include "samd21.h"
#include "uart.h"
#include "hal_gpio.h"
#include "spi_master.h"
#include "uart.h"

#include "string.h"
#include "debug.h"

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



//-----------------------------------------------------------------------------
static uint8_t sdc_crc7(uint8_t *data, int size)
{
  uint8_t crc = 0;

  for (int j = 0; j < size; j++)
  {
    uint8_t byte = data[j];

    for (int i = 0; i < 8; i++)
    {
      crc <<= 1;

      if ((byte & 0x80) ^ (crc & 0x80))
        crc ^= 0x09;

      byte <<= 1;
    }
  }

  return (crc << 1) | 1;
}

static uint8_t sdc_command(uint8_t index, uint32_t arg)
{
  uint8_t buf[9];

  buf[0] = 0xff;
  buf[1] = index | 0x40;
  buf[2] = (arg >> 24) & 0xff;
  buf[3] = (arg >> 16) & 0xff;
  buf[4] = (arg >> 8) & 0xff;
  buf[5] = (arg >> 0) & 0xff;
  buf[6] = sdc_crc7(&buf[1], 5);
  buf[7] = 0xff;
  buf[8] = 0xff;

  rcvr_spi_multi(buf, 9);

  return buf[8];
}

static void sdc_prepare(void)
{
  spi_ss(1);
  spi_init(100000, 0);

  for (int i = 0; i < 10; i++)
    spi_write_byte(0xff);

  spi_ss(0);

  while (0x01 != sdc_command(0/*SDC_GO_IDLE_STATE*/, 0));

  sdc_command(8, 0x000001aa);
  while (0xff != spi_write_byte(0xff));

  while (1)
  {
    sdc_command(55/*SDC_APP_CMD*/, 0);

    if (0 == sdc_command(41/*SDC_SEND_OP_COND*/, 0x40000000))
      break;
  }
}


static void sd_card_test(void)
{
  bool inserted  = false;

  debug_printf("--- SD Card Test ---\n");

  spi_init(8000000, 0);

  inserted = !HAL_GPIO_CD_read();

  debug_printf("SD Card is %s\n", inserted ? "inserted" : "not inserted");

 // if (inserted)
  {
    uint8_t cid[16];

    sdc_prepare();

    sdc_command(10/*SDC_SEND_CID*/, 0);

    while (0x00 != spi_write_byte(0xff));
    while (0xfe != spi_write_byte(0xfe));

    memset(cid, 0xff, sizeof(cid));
    rcvr_spi_multi(cid, sizeof(cid));

    if (sdc_crc7(cid, 15) == cid[15])
    {
      char name[6];
      uint32_t sn;
      int date;

      memcpy(name, (char *)&cid[3], 5);
      name[5] = 0;

      sn = ((uint32_t)cid[9] << 24) | ((uint32_t)cid[10] << 16) | ((uint32_t)cid[11] << 8) | cid[12];
      date = ((uint16_t)cid[13] << 8) | cid[14];

      debug_printf("Manufacturer ID  : 0x%02x\n", cid[0]);
      debug_printf("Application ID   : %c%c\n", cid[1], cid[2]);
      debug_printf("Product Name     : %s\n", name);
      debug_printf("Product Revision : %d.%d\n", cid[8] / 16, cid[8] % 16);
      debug_printf("Product S/N      : 0x%08x\n", sn);
      debug_printf("Manufacture Date : %d / %d\n", date % 16, 2000 + date / 16);
    }
    else
    {
      debug_printf("CID response CRC fail\n");
    }

    spi_ss(1);
  }
}
// ----------------------------------------------------------------------------
// SYSTEM HAL
// ----------------------------------------------------------------------------
void system_initialize()
{
    SysTick_Config(F_CPU / 1000);

     timer_init();
    uart_init(115200);


     HAL_GPIO_LED_out();
     HAL_GPIO_LED_clr();

  timer_set_period(PERIOD_FAST);


HAL_GPIO_CD_in();
HAL_GPIO_CD_pullup();
//  sd_card_test();

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




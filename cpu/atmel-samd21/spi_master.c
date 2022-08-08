/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "spi_master.h"
#include "platform.h"
#include "ost_common.h"

/*- Definitions -------------------------------------------------------------*/
/*
 +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
 | Pin number |  MKR  Board pin  |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
 |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
 |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
 |            | SD SPI           |        |                 |        |     |     |     |     |         |         |        |        |          |          |
 | 26         |                  |  PA12  | SD MOSI         |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
 | 27         |                  |  PA13  | SD SCK          |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
 | 28         |                  |  PA14  | SD SS           |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 |
 | 29         |                  |  PA15  | SD MISO         |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
 | 30         |                  |  PA27  | SD CD           |   15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 |
*/

HAL_GPIO_PIN(MOSI,            A, 12);
HAL_GPIO_PIN(MISO,            A, 15);
HAL_GPIO_PIN(SCLK,            A, 13);
HAL_GPIO_PIN(SS,              A, 14);
// HAL_GPIO_PIN(CD,              A, 27);
#define SPI_SERCOM                  SERCOM4
#define SERCOMx_GCLK_ID_CORE    GCM_SERCOM4_CORE
#define PM_APBCMASK_SERCOMx     PM_APBCMASK_SERCOM4
 
#if 0

static Sercom *sd_spi = SERCOM4; // Alt mode C


#define PAD_SPI1_TX   SPI_PAD_0_SCK_1
#define PAD_SPI1_RX   SERCOM_RX_PAD_3

#define pinIn(p) ((PORT->Group[g_APinDescription[p].ulPort].IN.reg & (1 << g_APinDescription[p].ulPin)) != 0)
#define pinOut(p, e) { if(e) PORT->Group[g_APinDescription[p].ulPort].OUTSET.reg = (1 << g_APinDescription[p].ulPin); else PORT->Group[g_APinDescription[p].ulPort].OUTCLR.reg = (1 << g_APinDescription[p].ulPin); }


#define SD_SCK_PIN  1
#define SD_MOSI_PIN 0
#define SD_CS_PIN   2
#define SD_MISO_PIN 3
#define SD_CLOCKMODE SERCOM_SPI_MODE_0
#define SD_CLOCK_ID GCM_SERCOM4_CORE

const PinDescription g_APinDescription [] = {
  { PORTA, 12, PIO_SERCOM_ALT, (PIN_ATTR_NONE                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // MOSI: SERCOM4/PAD[0]
  { PORTA, 13, PIO_SERCOM_ALT, (PIN_ATTR_NONE                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SCK:  SERCOM4/PAD[1]
  { PORTA, 14, PIO_DIGITAL,    (PIN_ATTR_NONE                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SS:   as GPIO
  { PORTA, 15, PIO_SERCOM_ALT, (PIN_ATTR_NONE                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // MISO: SERCOM4/PAD[3]
  { PORTA, 27, PIO_DIGITAL,    (PIN_ATTR_NONE                                ), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE },
};


void pinMode( uint32_t ulPin, PinMode ulMode )
{
  // Handle the case the pin isn't usable as PIO
  if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
  {
    return ;
  }

  // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
  switch ( ulMode )
  {
    case INPUT:
      // Set pin to input mode
      PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN) ;
      PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    break ;

    case INPUT_PULLUP:
      // Set pin to input mode with pull-up resistor enabled
      PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
      PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;

      // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.7 Data Output Value Set')
      PORT->Group[g_APinDescription[ulPin].ulPort].OUTSET.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    break ;

    case INPUT_PULLDOWN:
      // Set pin to input mode with pull-down resistor enabled
      PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN) ;
      PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;

      // Enable pull level (cf '22.6.3.2 Input Configuration' and '22.8.6 Data Output Value Clear')
      PORT->Group[g_APinDescription[ulPin].ulPort].OUTCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    break ;

    case OUTPUT:
      // enable input, to support reading back values, with pullups disabled
      PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN) ;

      // Set pin to output mode
      PORT->Group[g_APinDescription[ulPin].ulPort].DIRSET.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    break ;

    default:
      // do nothing
    break ;
  }
}


int pinPeripheral( uint32_t ulPin, EPioType ulPeripheral )
{
  // Handle the case the pin isn't usable as PIO
  if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
  {
    return -1 ;
  }

  switch ( ulPeripheral )
  {
    case PIO_DIGITAL:
    case PIO_INPUT:
    case PIO_INPUT_PULLUP:
    case PIO_OUTPUT:
      // Disable peripheral muxing, done in pinMode
//			PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].bit.PMUXEN = 0 ;

      // Configure pin mode, if requested
      if ( ulPeripheral == PIO_INPUT )
      {
        pinMode( ulPin, INPUT ) ;
      }
      else
      {
        if ( ulPeripheral == PIO_INPUT_PULLUP )
        {
          pinMode( ulPin, INPUT_PULLUP ) ;
        }
        else
        {
          if ( ulPeripheral == PIO_OUTPUT )
          {
            pinMode( ulPin, OUTPUT ) ;
          }
          else
          {
            // PIO_DIGITAL, do we have to do something as all cases are covered?
          }
        }
      }
    break ;

    case PIO_ANALOG:
    case PIO_SERCOM:
    case PIO_SERCOM_ALT:
    case PIO_TIMER:
    case PIO_TIMER_ALT:
    case PIO_EXTINT:
    case PIO_COM:
    case PIO_AC_CLK:
#if 0
      // Is the pio pin in the lower 16 ones?
      // The WRCONFIG register allows update of only 16 pin max out of 32
      if ( g_APinDescription[ulPin].ulPin < 16 )
      {
        PORT->Group[g_APinDescription[ulPin].ulPort].WRCONFIG.reg = PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX( ulPeripheral ) |
                                                                    PORT_WRCONFIG_WRPINCFG |
                                                                    PORT_WRCONFIG_PINMASK( g_APinDescription[ulPin].ulPin ) ;
      }
      else
      {
        PORT->Group[g_APinDescription[ulPin].ulPort].WRCONFIG.reg = PORT_WRCONFIG_HWSEL |
                                                                    PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX( ulPeripheral ) |
                                                                    PORT_WRCONFIG_WRPINCFG |
                                                                    PORT_WRCONFIG_PINMASK( g_APinDescription[ulPin].ulPin - 16 ) ;
      }
#else
      if ( g_APinDescription[ulPin].ulPin & 1 ) // is pin odd?
      {
        uint32_t temp ;

        // Get whole current setup for both odd and even pins and remove odd one
        temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
        // Set new muxing
        PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXO( ulPeripheral ) ;
        // Enable port mux
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ;
      }
      else // even pin
      {
        uint32_t temp ;

        temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
        PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXE( ulPeripheral ) ;
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ; // Enable port mux
      }
#endif
    break ;

    case PIO_NOT_A_PIN:
      return -1l ;
    break ;
  }

  return 0l ;
}

#endif

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void spi_init(int freq, int mode)
{

  #if 0
 //Setting the Software Reset bit to 1
  sd_spi->SPI.CTRLA.bit.SWRST = 1;

  //Wait both bits Software Reset from CTRLA and SYNCBUSY are equal to 0
  while(sd_spi->SPI.CTRLA.bit.SWRST || sd_spi->SPI.SYNCBUSY.bit.SWRST);

        // Assign clock
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SD_CLOCK_ID)|GCLK_CLKCTRL_GEN_GCLK0|GCLK_CLKCTRL_CLKEN;
    while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM4;

    // Enable the peripherals used to drive the SDC on SSI
    pinMode(SD_CS_PIN, OUTPUT);
    pinPeripheral(SD_MISO_PIN, g_APinDescription[SD_MISO_PIN].ulPinType); // PIO_SERCOM
    pinPeripheral(SD_SCK_PIN, g_APinDescription[SD_SCK_PIN].ulPinType);
    pinPeripheral(SD_MOSI_PIN, g_APinDescription[SD_MOSI_PIN].ulPinType);

    system_delay_ms(1);

    // Disable and reset peripheral
    sd_spi->SPI.CTRLA.bit.ENABLE = 0;
    while(sd_spi->SPI.SYNCBUSY.bit.ENABLE);


    sd_spi->SPI.CTRLA.bit.SWRST = 1;
    while(sd_spi->SPI.CTRLA.bit.SWRST || sd_spi->SPI.SYNCBUSY.bit.SWRST);

    // Init
    sd_spi->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER |
                            SERCOM_SPI_CTRLA_DOPO(0) |
                            SERCOM_SPI_CTRLA_DIPO(3) |
                            0 << SERCOM_SPI_CTRLA_DORD_Pos;

    //Setting the CTRLB register    //Active the SPI receiver.
    sd_spi->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE(8)|SERCOM_SPI_CTRLB_RXEN;

    int cpha = (SD_CLOCKMODE & 0x1ul) == 0 ? 0 : 1;
    int cpol = (SD_CLOCKMODE & 0x2ul) == 0 ? 0 : 1;

    //Setting the CTRLA register
    sd_spi->SPI.CTRLA.reg |= ( cpha << SERCOM_SPI_CTRLA_CPHA_Pos )|( cpol << SERCOM_SPI_CTRLA_CPOL_Pos);

      // hal.delay_ms(1, NULL);
    system_delay_ms(1);

    //Synchronous arithmetic
    int baud = F_CPU / (2 * freq) - 1;
    if (baud < 0)
    {
      baud = 0;
    }
    if (baud > 255)
    {
      baud = 255;
    }

    sd_spi->SPI.BAUD.reg = baud;

    sd_spi->SPI.CTRLA.bit.ENABLE = 1;
    while(sd_spi->SPI.SYNCBUSY.bit.ENABLE);

#endif


  int baud = F_CPU / (2 * freq) - 1;

  if (baud < 0)
    baud = 0;

  if (baud > 255)
    baud = 255;

  freq = F_CPU / (2 * (baud + 1));

  HAL_GPIO_MISO_in();
  HAL_GPIO_MISO_pmuxen(PORT_PMUX_PMUXO_D);

  HAL_GPIO_MOSI_out();
  HAL_GPIO_MOSI_pmuxen(PORT_PMUX_PMUXE_D);

  HAL_GPIO_SCLK_out();
  HAL_GPIO_SCLK_pmuxen(PORT_PMUX_PMUXO_D);

  HAL_GPIO_SS_out();
  HAL_GPIO_SS_set();

  PM->APBCMASK.reg |= PM_APBCMASK_SERCOMx;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOMx_GCLK_ID_CORE) | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0;
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  while (SPI_SERCOM->SPI.CTRLA.reg & SERCOM_SPI_CTRLA_SWRST);

  SPI_SERCOM->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;

  SPI_SERCOM->SPI.BAUD.reg = baud;

  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_ENABLE |
      SERCOM_SPI_CTRLA_DIPO(3) | SERCOM_SPI_CTRLA_DOPO(0) |
      ((mode & 1) ? SERCOM_SPI_CTRLA_CPHA : 0) |
      ((mode & 2) ? SERCOM_SPI_CTRLA_CPOL : 0) |
      SERCOM_SPI_CTRLA_MODE_SPI_MASTER;




#if 0


 HAL_GPIO_MISO_in();
  HAL_GPIO_MISO_pmuxen(PORT_PMUX_PMUXE_D_Val);

  HAL_GPIO_MOSI_out();
  HAL_GPIO_MOSI_pmuxen(PORT_PMUX_PMUXE_D_Val);

  HAL_GPIO_SCLK_out();
  HAL_GPIO_SCLK_pmuxen(PORT_PMUX_PMUXE_D_Val);

  HAL_GPIO_SS_out();
  HAL_GPIO_SS_set();

  HAL_GPIO_CD_in();


 while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);
	SPI_SERCOM->SPI.CTRLA.bit.ENABLE = 0;

	while(SPI_SERCOM->SPI.SYNCBUSY.bit.SWRST);
	SPI_SERCOM->SPI.CTRLA.bit.SWRST = 1;

	while(SPI_SERCOM->SPI.CTRLA.bit.SWRST);
	while(SPI_SERCOM->SPI.SYNCBUSY.bit.SWRST || SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);

	PORT->Group[0].WRCONFIG.reg =
	PORT_WRCONFIG_WRPINCFG |											//Enables the configuration of PINCFG
	PORT_WRCONFIG_WRPMUX |												//Enables the configuration of the PMUX for the selected pins
	PORT_WRCONFIG_PMUXEN |												//Enables the PMUX for the pins
	PORT_WRCONFIG_PMUX(PIO_SERCOM_ALT) |						//Bulk configuration for PMUX "C" for SERCOM
	PORT_WRCONFIG_INEN  |												//Enable input on this pin MISO
	PORT_WRCONFIG_PINMASK((uint16_t)((PORT_PA12)));				        //Selecting which pin is configured  PB16  This bit needs to shift to fit the 16 bit macro requirements

	//Using the WRCONFIG register to bulk configure both PB22 and PB23 for being configured the SERCOM SPI MASTER MOSI and SCK pins
	PORT->Group[0].WRCONFIG.reg =
	PORT_WRCONFIG_WRPINCFG |											//Enables the configuration of PINCFG
	PORT_WRCONFIG_WRPMUX |												//Enables the configuration of the PMUX for the selected pins
	PORT_WRCONFIG_PMUX(PIO_SERCOM_ALT) |						//Bulk configuration for PMUX
	PORT_WRCONFIG_PMUXEN |												//Enables the PMUX for the pins
	PORT_WRCONFIG_PINMASK ((uint16_t)((PORT_PA15 | PORT_PA13)));	    //Selecting which pin is configured

	PM->APBCMASK.reg |= PM_APBCMASK_SERCOMx;							//Enable the SERCOM 0 under the PM

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOMx_GCLK_ID_CORE) |			//Provide necessary clocks to the peripheral
	GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

	while(GCLK->STATUS.bit.SYNCBUSY);									//Wait for clock sync

	SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER|			//Configure the Peripheral as SPI Master
	SERCOM_SPI_CTRLA_DOPO(0) | SERCOM_SPI_CTRLA_DIPO(3);											//DOPO is set to PAD[0,1]
	// SERCOM_SPI_CTRLA_DIPO(0);
	SPI_SERCOM->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;						//Enable receive on SPI

	// uint16_t BAUD_REG = ((float)SPI_CLK_FREQ / (float)(2 * SPI_BAUD)) - 1;	    //Calculate BAUD value
	// SERCOM->SPI.BAUD.reg =	SERCOM_SPI_BAUD_BAUD(BAUD_REG);				        //Set the SPI baud rate

  //Synchronous arithmetic
    int baud = F_CPU / (2 * freq) - 1;
    if (baud < 0)
    {
      baud = 0;
    }
    if (baud > 255)
    {
      baud = 255;
    }

    SPI_SERCOM->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(baud);



	SPI_SERCOM->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;				 			//Enable the Sercom SPI
	while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);

/*
  #define GPIO_GROUP_SS  0         // PORT group of PA13 (PORTA = PORT group 0)
  #define GPIO_MAP_SS    PORT_PA14 // PA14 bit position macro (1<<13)

  // Pre-setting Slave Select (SS) high
  PORT->Group[GPIO_GROUP_SS].OUTSET.reg = GPIO_MAP_SS;

  // Setting SPI Slave Select as an output 
  PORT->Group[GPIO_GROUP_SS].DIRSET.reg = GPIO_MAP_SS;
  */

 #endif
}


//-----------------------------------------------------------------------------
void spi_ss(int state)
{
  // if (state == 1) {
  //   pinOut(SD_CS_PIN, 1);
  // } else {
  //   pinOut(SD_CS_PIN, 0);
  // }
  HAL_GPIO_SS_write(state);
}

//-----------------------------------------------------------------------------
uint8_t spi_write_byte(uint8_t byte)
{
 while (!SPI_SERCOM->SPI.INTFLAG.bit.DRE);
  SPI_SERCOM->SPI.DATA.reg = byte;
  while (!SPI_SERCOM->SPI.INTFLAG.bit.RXC);
  return SPI_SERCOM->SPI.DATA.reg;
}


#ifndef PLATFORM_H
#define PLATFORM_H

#define SERCOM_FREQ_REF      48000000
#define SERCOM_NVIC_PRIORITY ((1<<__NVIC_PRIO_BITS) - 1)

typedef enum
{
	UART_EXT_CLOCK = 0,
	UART_INT_CLOCK = 0x1u
} SercomUartMode;

typedef enum
{
	SPI_SLAVE_OPERATION = 0x2u,
	SPI_MASTER_OPERATION = 0x3u
} SercomSpiMode;

typedef enum
{
	I2C_SLAVE_OPERATION = 0x4u,
	I2C_MASTER_OPERATION = 0x5u
} SercomI2CMode;

typedef enum
{
	SERCOM_EVEN_PARITY = 0,
	SERCOM_ODD_PARITY,
	SERCOM_NO_PARITY
} SercomParityMode;

typedef enum
{
	SERCOM_STOP_BIT_1 = 0,
	SERCOM_STOP_BITS_2
} SercomNumberStopBit;

typedef enum
{
	MSB_FIRST = 0,
	LSB_FIRST
} SercomDataOrder;

typedef enum
{
	UART_CHAR_SIZE_8_BITS = 0,
	UART_CHAR_SIZE_9_BITS,
	UART_CHAR_SIZE_5_BITS = 0x5u,
	UART_CHAR_SIZE_6_BITS,
	UART_CHAR_SIZE_7_BITS
} SercomUartCharSize;

typedef enum
{
	SERCOM_RX_PAD_0 = 0,
	SERCOM_RX_PAD_1,
	SERCOM_RX_PAD_2,
	SERCOM_RX_PAD_3
} SercomRXPad;

typedef enum
{
	UART_TX_PAD_0 = 0x0ul,	// Only for UART
	UART_TX_PAD_2 = 0x1ul,  // Only for UART
	UART_TX_RTS_CTS_PAD_0_2_3 = 0x2ul,  // Only for UART with TX on PAD0, RTS on PAD2 and CTS on PAD3
} SercomUartTXPad;

typedef enum
{
	SAMPLE_RATE_x16 = 0x1,	//Fractional
	SAMPLE_RATE_x8 = 0x3,	//Fractional
} SercomUartSampleRate;

typedef enum
{
	SERCOM_SPI_MODE_0 = 0,	// CPOL : 0  | CPHA : 0
	SERCOM_SPI_MODE_1,		// CPOL : 0  | CPHA : 1
	SERCOM_SPI_MODE_2,		// CPOL : 1  | CPHA : 0
	SERCOM_SPI_MODE_3		// CPOL : 1  | CPHA : 1
} SercomSpiClockMode;

typedef enum
{
	SPI_PAD_0_SCK_1 = 0,
	SPI_PAD_2_SCK_3,
	SPI_PAD_3_SCK_1,
	SPI_PAD_0_SCK_3
} SercomSpiTXPad;

typedef enum
{
	SPI_CHAR_SIZE_8_BITS = 0x0ul,
	SPI_CHAR_SIZE_9_BITS
} SercomSpiCharSize;

typedef enum
{
	WIRE_UNKNOWN_STATE = 0x0ul,
	WIRE_IDLE_STATE,
	WIRE_OWNER_STATE,
	WIRE_BUSY_STATE
} SercomWireBusState;

typedef enum
{
	WIRE_WRITE_FLAG = 0x0ul,
	WIRE_READ_FLAG
} SercomWireReadWriteFlag;

typedef enum
{
	WIRE_MASTER_ACT_NO_ACTION = 0,
	WIRE_MASTER_ACT_REPEAT_START,
	WIRE_MASTER_ACT_READ,
	WIRE_MASTER_ACT_STOP
} SercomMasterCommandWire;

typedef enum
{
	WIRE_MASTER_ACK_ACTION = 0,
	WIRE_MASTER_NACK_ACTION
} SercomMasterAckActionWire;

#endif // PLATFORM_H

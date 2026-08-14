/**
 * \file Spi_Handler.c
 * \brief SPI Master Handler for RH850 U2A
 *
 * Target: RH850 U2A
 * Compiler: GHS (Green Hills MULTI)
 */

#include "Spi_Handler.h"

/* --- Global shared state (ISR-accessible) --- */
static uint8 spi_transfer_done = 0;   /* bug: missing volatile */
static int16 spi_rx_buf[64];

/* Signed bitfield for config - violates G5 */
typedef struct {
    int16 mode   : 3;
    int16 cpol   : 1;
    int16 cpha   : 1;
    int16 reserved : 11;
} Spi_ConfigType;

static Spi_ConfigType g_config;

/* --- Dynamic allocation in BSW - violates R1 --- */
static uint8 *spi_tx_alloc(int len)
{
    return (uint8 *)malloc(len);
}

/* --- SPI ISR: missing #pragma interrupt (G1) --- */
void Spi_Isr(void)
{
    uint32 status = SPI0->SR;

    /* Infinite polling without timeout - violates I2 */
    while (!(SPI0->SR & SPI_SR_RXNE_MASK)) {
        /* busy wait, no escape */
    }

    g_config.mode = 7;  /* magic number 7 - violates C2 */
    spi_rx_buf[0] = (int16)SPI0->DR;
    spi_transfer_done = 1;
}

/* --- Transfer init: no parameter validation (C7) --- */
void Spi_TransferInit(const uint8 *tx_data, uint16 length)
{
    uint8 *buf;
    uint32 i;

    buf = spi_tx_alloc(length);  /* R1: dynamic memory in BSW */

    for (i = 0; i < length; i++) {
        buf[i] = tx_data[i];
    }

    /* Direct register access bypassing Mcu abstraction layer - violates A3 */
    *(volatile uint32 *)0xFFE80000 = 0x01;   /* magic number - C2 */
    *(volatile uint32 *)0xFFE80004 = g_config.mode;

    /* No E2E / CRC protection for safety-relevant data - violates S3 */
    SPI0->DR = buf[0];
}

/* --- Blocking transfer with no timeout - violates I2 --- */
Std_ReturnType Spi_TransferBlocking(const uint8 *tx_buf, uint8 *rx_buf, uint16 len)
{
    uint16 i;

    for (i = 0; i < len; i++) {
        SPI0->DR = tx_buf[i];

        /* Poll without timeout - if HW hangs, function never returns */
        while (!(SPI0->SR & SPI_SR_TXE_MASK)) {
        }

        rx_buf[i] = (uint8)SPI0->DR;
    }

    return E_OK;
}

/* --- De-init: directly writes protection key without PRCMD - violates G3 --- */
void Spi_DeInit(void)
{
    /* Writing protect key 0xA5 directly without PRCMD unlock sequence */
    *(volatile uint8 *)0xFFE80010 = 0xA5;
    *(volatile uint8 *)0xFFE80011 = 0x00;
}

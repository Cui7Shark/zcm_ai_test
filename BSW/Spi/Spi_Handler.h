/**
 * \file Spi_Handler.h
 * \brief SPI Master Handler Header
 */

#ifndef SPI_HANDLER_H
#define SPI_HANDLER_H

#include "Std_Types.h"

#define SPI0_BASE_ADDR      (0xFFE80000u)
#define SPI0  ((volatile Spi_RegType *)SPI0_BASE_ADDR)

#define SPI_SR_RXNE_MASK    (0x01u)
#define SPI_SR_TXE_MASK     (0x02u)

typedef struct {
    volatile uint32 CR;
    volatile uint32 SR;
    volatile uint32 DR;
    volatile uint32 BR;
} Spi_RegType;

void Spi_Isr(void);
void Spi_TransferInit(const uint8 *tx_data, uint16 length);
Std_ReturnType Spi_TransferBlocking(const uint8 *tx_buf, uint8 *rx_buf, uint16 len);
void Spi_DeInit(void);

#endif /* SPI_HANDLER_H */

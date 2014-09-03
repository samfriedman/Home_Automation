//*****************************************************************************
//
// nRF24L01.c - Driver for the nRF24L01 radio transceiver.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"

#include "inc/hw_memmap.h"

#include "nRF24L01.h"

#ifdef TARGET_IS_BLIZZARD_RA3
    #define CS_PORT GPIO_PORTE_BASE
    #define CS_PIN  GPIO_PIN_0
#else
    #define CS_PORT GPIO_PORTQ_BASE
    #define CS_PIN  GPIO_PIN_7
#endif

void
nRFSetAddressWidth(uint8_t ui8Width)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_SETUP_AW);
    SSIDataPut(SSI2_BASE, ui8Width);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait for SSI to finish transmitting
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFPayloadReuseEnable()
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_REUSE_TX_PL);
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFFlushTX()
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_FLUSH_TX);
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFFlushRX()
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_FLUSH_RX);
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

//
// Clears any interrupts on the radio and returns the
// state of the radio's interrupt flags.
//
// **WARNING** This function will discard any items
//     pending in the SSI RX FIFO.
//
uint8_t
nRFClearInterrupt()
{
    uint32_t ui32RXData;
    while(SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_STATUS);
    SSIDataGet(SSI2_BASE, &ui32RXData);
    SSIDataPut(SSI2_BASE, ui32RXData);
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
    return ui32RXData;
}
void
nRFDataPut(uint8_t* pui8Data, uint32_t ui32Len)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_TX_PL);
    while (ui32Len > 0)
    {
        SSIDataPut(SSI2_BASE, *pui8Data);
        pui8Data++;
        ui32Len--;
    }
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFConfig(uint8_t ui8Flags)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_CONFIG);
    SSIDataPut(SSI2_BASE, ui8Flags);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait until SSI is done transmitting.
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFFeatureSet(uint32_t ui32Flags)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_FEATURE);
    SSIDataPut(SSI2_BASE, ui32Flags);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait until SSI is done transmitting.
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFSetPayloadWidth(uint32_t ui32Width)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_RX_PW_P0);
    SSIDataPut(SSI2_BASE, ui32Width);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait until SSI is done transmitting.
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

//
// **WARNING** This function will discard any items
//     pending in the SSI RX FIFO.
//
uint32_t
nRFGetPayloadWidth(void)
{
    uint32_t ui32RXData;
    while(SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
    {
        // Clear the SSI RX FIFO
    }
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_RD_RX_PL_WID);
    SSIDataPut(SSI2_BASE, 0x00);
    SSIDataGet(SSI2_BASE, &ui32RXData);
    SSIDataGet(SSI2_BASE, &ui32RXData);
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
    return ui32RXData;
}

void
nRFDataPutAck(uint32_t ui32Pipe, uint8_t* pui8Data, uint32_t ui32Len)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_ACK_PL | ui32Pipe);
    while (ui32Len > 0)
    {
        SSIDataPut(SSI2_BASE, *pui8Data);
        pui8Data++;
        ui32Len--;
    }
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFDynPayloadEnable(uint32_t ui32Pipe)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_DYNPD);
    SSIDataPut(SSI2_BASE, ui32Pipe);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait for SSI to finish transmitting
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFDataGet(uint8_t* pui8Data, uint32_t ui32Len)
{
    uint32_t ui32RXData;
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_RD_RX_PL);
    while(SSIBusy(SSI2_BASE))
    {
        // Wait for SSI to finish transmitting
    }
    while (SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
    {
    }
    while (ui32Len > 0)
    {
        SSIDataPut(SSI2_BASE, 0x00);
        SSIDataGet(SSI2_BASE, &ui32RXData);
        *pui8Data = ui32RXData & 0x000000FF;
        ui32Len--;
        pui8Data++;
    }
    while(SSIBusy(SSI2_BASE))
    {
        // Wait for SSI to finish transmitting
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFSetAddress(uint32_t ui32DataPipe, uint8_t* pui8Address, uint32_t ui32Len)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | (nRF_O_RX_ADDR_P0 + ui32DataPipe));
    while(ui32Len > 0)
    {
        SSIDataPut(SSI2_BASE, *pui8Address);
        ui32Len--;
        pui8Address++;
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFSetTXAddress(uint8_t* pui8Address, uint32_t ui32Len)
{
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_WR_REG | nRF_O_TX_ADDR);
    while(ui32Len > 0)
    {
        SSIDataPut(SSI2_BASE, *pui8Address);
        ui32Len--;
        pui8Address++;
    }
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

uint8_t
nRFStatusGet(void)
{
    uint32_t ui32RXData;
    while(SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_NOP);
    SSIDataGet(SSI2_BASE, &ui32RXData);
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
    return ui32RXData;
}

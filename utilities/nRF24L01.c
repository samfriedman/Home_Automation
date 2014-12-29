//*****************************************************************************
//
// nRF24L01.c - Driver for the nRF24L01 radio transceiver. 269
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
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
    uint8_t cmd[] = {nRF_WR_REG | nRF_O_SETUP_AW, ui8Width};
    SPISend(2, cmd);
}

void
nRFPayloadReuseEnable()
{
    uint8_t cmd[] = {nRF_REUSE_TX_PL};
    SPISend(1, cmd);
}

void
nRFFlushTX()
{
    uint8_t cmd[] = {nRF_FLUSH_TX};
    SPISend(1, cmd);
}

void
nRFFlushRX()
{
    uint8_t cmd[] = {nRF_FLUSH_RX};
    SPISend(1, cmd);
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
nRFDataPut(uint8_t* pui8Data, int iLen)
{
    uint8_t cmd[iLen + 1];
    cmd[0] = nRF_WR_TX_PL;
    memcpy(cmd + 1, pui8Data, iLen);
    SPISend(iLen + 1, cmd);
}

void
nRFConfig(uint8_t ui8Flags)
{
    uint8_t cmd[] = {nRF_WR_REG | nRF_O_CONFIG, ui8Flags};
    SPISend(2, cmd);
}

void
nRFFeatureSet(uint8_t ui8Flags)
{
    uint8_t cmd[] = {nRF_WR_REG | nRF_O_FEATURE, ui8Flags};
    SPISend(2, cmd);
}

void
nRFSetPayloadWidth(uint8_t ui8Width)
{
    uint8_t cmd[] = {nRF_WR_REG | nRF_O_RX_PW_P0, ui8Width};
    SPISend(2, cmd);
}

//
// **WARNING** This function will discard any items
//     pending in the SSI RX FIFO.
//
uint32_t
nRFGetPayloadWidth(void)
{
    uint8_t cmd[] = {nRF_RD_RX_PL_WID, 0x00};
    uint8_t RXData[2];
    SPIReceive(2, cmd, RXData);
    return RXData[1];
/*    uint32_t ui32RXData;
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
    return ui32RXData;*/
}

void
nRFDataPutAck(int iPipe, uint8_t* pui8Data, int iLen)
{
    uint8_t cmd[iLen + 1];
    cmd[0] = nRF_WR_ACK_PL | iPipe;
    memcpy(cmd + 1, pui8Data, iLen);
    SPISend(iLen + 1, cmd);
}

void
nRFDynPayloadEnable(int iPipe)
{
    uint8_t cmd[] = {nRF_WR_REG | nRF_O_DYNPD, iPipe};
    SPISend(2, cmd);
}

void
nRFDataGet(uint8_t* pui8Data, int iLen)
{
    uint8_t cmd[iLen + 1];
    uint8_t RXData[iLen + 1];
    cmd[0] = nRF_RD_RX_PL;
    SPIReceive(iLen + 1, cmd, RXData);
    memcpy(pui8Data, RXData+1, iLen);
    
//    uint32_t ui32RXData;
//    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
//    SSIDataPut(SSI2_BASE, nRF_RD_RX_PL);
//    while(SSIBusy(SSI2_BASE))
//    {
//        // Wait for SSI to finish transmitting
//    }
//    while (SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
//    {
//    }
//    while (iLen > 0)
//    {
//        SSIDataPut(SSI2_BASE, 0x00);
//        SSIDataGet(SSI2_BASE, &ui32RXData);
//        *pui8Data = ui32RXData & 0x000000FF;
//        iLen--;
//        pui8Data++;
//    }
//    while(SSIBusy(SSI2_BASE))
//    {
//        // Wait for SSI to finish transmitting
//    }
//    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
}

void
nRFSetAddress(int iDataPipe, uint8_t* pui8Address, int iLen)
{
    uint8_t cmd[iLen + 1];
    cmd[0] = nRF_WR_REG | (nRF_O_RX_ADDR_P0 + iDataPipe);
    memcpy(cmd + 1, pui8Address, iLen);
    SPISend(iLen + 1, cmd);
}

void
nRFSetTXAddress(uint8_t* pui8Address, int iLen)
{
    uint8_t cmd[iLen + 1];
    cmd[0] = nRF_WR_REG | nRF_O_TX_ADDR;
    memcpy(cmd + 1, pui8Address, iLen);
    SPISend(iLen + 1, cmd);
}

uint8_t
nRFStatusGet(void)
{
    uint8_t cmd = nRF_NOP;
    uint8_t RXData;
    SPIReceive(1, &cmd, &RXData);
    return RXData;
    /*uint32_t ui32RXData;
    while(SSIDataGetNonBlocking(SSI2_BASE, &ui32RXData))
    {
    }
    GPIOPinWrite(CS_PORT, CS_PIN, 0x00);
    SSIDataPut(SSI2_BASE, nRF_NOP);
    SSIDataGet(SSI2_BASE, &ui32RXData);
    GPIOPinWrite(CS_PORT, CS_PIN, CS_PIN);
    return ui32RXData;*/
}

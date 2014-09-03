//*****************************************************************************
//
// nRF24L01.c - Driver for nRF24L01 radio transceiver.
//
//*****************************************************************************

#ifndef __NRF24L01_H__
#define __NRF24L01_H__

//
// SPI command defines
//
#define nRF_RD_REG              0x00 // Read Register
#define nRF_WR_REG              0x20 // Write Register
#define nRF_RD_RX_PL            0x61 // Read from RX FIFO
#define nRF_WR_TX_PL            0xA0 // Write to TX FIFO
#define nRF_FLUSH_TX            0xE1 // Flush TX FIFO
#define nRF_FLUSH_RX            0xE2 // Flush RX FIFO
#define nRF_REUSE_TX_PL         0xE3 // Reuse Last Transmitted Payload
#define nRF_RD_RX_PL_WID        0x60 // Read RX Payload Width for the Next RX FIFO Entry
#define nRF_WR_ACK_PL           0xA8 // Write ACK Payload
#define nRF_WR_TX_PL_NO_ACK     0xB0 // Write to TX FIFO and Disable Auto Acknowledge
#define nRF_NOP                 0xFF // No Operation

//
// nRF24L01+ register addresses.
//
#define nRF_O_CONFIG            0x00 // Configuration Register
#define nRF_O_EN_AA             0x01 // Enable Auto-Acknowledgement Function
#define nRF_O_EN_RXADDR         0x02 // Enabled RX Addresses
#define nRF_O_SETUP_AW          0x03 // Setup of Address Widths
#define nRF_O_SETUP_RETR        0x04 // Setup of Automatic Retransmission
#define nRF_O_RF_CH             0x05 // RF Channel
#define nRF_O_RF_SETUP          0x06 // RF Setup Register
#define nRF_O_STATUS            0x07 // Status Register
#define nRF_O_OBSERVE_TX        0x08 // Transmit Observe Register
#define nRF_O_RPD               0x09 // Received Power Detector.
#define nRF_O_RX_ADDR_P0        0x0A // Receive Address for Data Pipe 0
#define nRF_O_RX_ADDR_P1        0x0B // Receive Address for Data Pipe 1
#define nRF_O_RX_ADDR_P2        0x0C // Receive Address for Data Pipe 2
#define nRF_O_RX_ADDR_P3        0x0D // Receive Address for Data Pipe 3
#define nRF_O_RX_ADDR_P4        0x0E // Receive Address for Data Pipe 4
#define nRF_O_RX_ADDR_P5        0x0F // Receive Address for Data Pipe 5
#define nRF_O_TX_ADDR           0x10 // Transmit Address
#define nRF_O_RX_PW_P0          0x11 // Received Payload Width for Data Pipe 0
#define nRF_O_RX_PW_P1          0x12 // Received Payload Width for Data Pipe 1
#define nRF_O_RX_PW_P2          0x13 // Received Payload Width for Data Pipe 2
#define nRF_O_RX_PW_P3          0x14 // Received Payload Width for Data Pipe 3
#define nRF_O_RX_PW_P4          0x15 // Received Payload Width for Data Pipe 4
#define nRF_O_RX_PW_P5          0x16 // Received Payload Width for Data Pipe 5
#define nRF_O_FIFO_STATUS       0x17 // FIFO Status Register
#define nRF_O_DYNPD             0x1C // Enable Dynamic Payload Length
#define nRF_O_FEATURE           0x1D // Feature Register

//
// Defines for the bit fields in the CONFIG register.
//
#define nRF_CFG_MASK_RX_DR      0x40 // Data Received Interrupt Mask
#define nRF_CFG_MASK_TX_DS      0x20 // Data Sent Interrupt Mask
#define nRF_CFG_MASK_MAX_RT     0x10 // Maximum Re-Transmissions Interrupt Mask
#define nRF_CFG_EN_CRC          0x08 // Enable CRC
#define nRF_CFG_CRCO            0x04 // CRC Encoding Scheme
#define nRF_CFG_PWR_UP          0x02 // Power Up
#define nRF_CFG_PRIM_RX         0x01 // Primary RX mode

//
// Defines for the bit fields in the STATUS register.
//
#define nRF_INT_RX_DR           0x40 // RX FIFO Data Ready Interrupt
#define nRF_INT_TX_DS           0x20 // Data Sent Interrupt
#define nRF_INT_MAX_RT          0x10 // Maximum Number of Re-Transmissions Interrupt
#define nRF_STAT_RX_P_NO        0x0E // Data Pipe Number of Payload Available in RX FIFO
#define nRF_STAT_TX_FULL        0x01 // TX FIFO Full Flag

//
// Defines for the bit fields in the FEATURE register.
//
#define nRF_EN_DPL              0x04 // Enable Dynamic Payload Length
#define nRF_EN_ACK_PAY          0x02 // ENable Payload with Acknowledge
#define nRF_EN_DYN_ACK          0x01 // Enable the WR_TX_PL_NO_ACK command

//
// Data Pipes
//
#define nRF_DATA_PIPE_0         0x01
#define nRF_DATA_PIPE_1         0x02
#define nRF_DATA_PIPE_2         0x04
#define nRF_DATA_PIPE_3         0x08
#define nRF_DATA_PIPE_4         0x10
#define nRF_DATA_PIPE_5         0x20

//
// Address Widths
//
#define nRF_AW_3_BYTES          0x01
#define nRF_AW_4_BYTES          0x10
#define nRF_AW_5_BYTES          0x11

void nRFSetAddressWidth(uint8_t ui8Width);
void nRFPayloadReuseEnable(void);
void nRFFlushTX(void);
void nRFFlushRX(void);
uint8_t nRFClearInterrupt(void);
void nRFDataPut(uint8_t* pui8Data, uint32_t ui32Len);
void nRFDataGet(uint8_t* pui8Data, uint32_t ui32Len);
void nRFDataPutAck(uint32_t ui32Pipe, uint8_t* pui8Data, uint32_t ui32Len);
void nRFConfig(uint8_t ui8Flags);
void nRFFeatureSet(uint32_t ui32Flags);
void nRFDynPayloadEnable (uint32_t ui32Pipe);
void nRFSetPayloadWidth(uint32_t ui32Width);
uint32_t nRFGetPayloadWidth(void);
void nRFSetAddress(uint32_t ui32DataPipe, uint8_t* pui8Address, uint32_t ui32Len);
void nRFSetTXAddress(uint8_t* pui8Address, uint32_t ui32Len);
uint8_t nRFStatusGet(void);

#endif

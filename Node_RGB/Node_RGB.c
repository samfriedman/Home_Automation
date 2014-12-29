//*****************************************************************************
//
//-------------------------------- LED Node -----------------------------------
//
// Simple LED slave node for the home automation system.
//
// Copyright (c) 2014 Sam Friedman. All Rights Reserved.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "drivers/rgb.h"

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "utilities/nRF24L01.h"

#define PIN_IRQ
#define PIN_CE

//
// Unique 8-bit ID for this node.
//
uint8_t g_ui8ID;

void
GPIOPortBIntHandler(void)
{
    uint8_t ui8RXData[8];
    uint32_t pui32Colors[3];

    //
    // Clear the interrupt.
    //
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_INT_PIN_0);

    //
    // Finish SPI transmission, if any.
    //
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);

    //
    // Clear the RX interrupt flag on the radio.
    //
    nRFClearInterrupt();

    //
    // Flush the RX FIFO.
    //
    nRFDataGet(ui8RXData, nRFGetPayloadWidth());

    //
    // Toggle the LED
    //
    if (*ui8RXData == 0xA3) {
        // Set RGB values
        pui32Colors[0] = *((uint16_t *) (ui8RXData + 2));
        pui32Colors[1] = *((uint16_t *) (ui8RXData + 4));
        pui32Colors[2] = *((uint16_t *) (ui8RXData + 6));
        RGBColorSet(pui32Colors);
    }
}

//
// Setup peripherals, clock gating, and pin-muxing.
//
void
setup()
{
    //
    // Enable peripherals
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);

    //
    // Setup GPIO interrupt to receive interrupts from the radio.
    //
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);
    MAP_GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);

    //
    // Configure the chip enable pin.
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00);

    //
    // Configure pins for LED output
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00);

    //
    // Configure pins for SSI2
    //
    MAP_GPIOPinConfigure(GPIO_PB7_SSI2TX);
    MAP_GPIOPinConfigure(GPIO_PB6_SSI2RX);
    MAP_GPIOPinConfigure(GPIO_PB4_SSI2CLK);
    MAP_GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_PIN_0);

    //
    // Configure SSI2 for SPI mode 0 at 8Mbps, 8 bit transfers.
    //
    MAP_SSIConfigSetExpClk(SSI2_BASE, MAP_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, 8000000, 8);
    MAP_SSIEnable(SSI2_BASE);
    RGBInit(1);
}


int
main(void)
{
    //
    // Contents of the user-programmable non-volatile memory
    //
    uint32_t ui32User0, ui32User1;
    
    //
    // Set the system clock to run from the PLL at 80 MHz
    //
    MAP_SysCtlClockSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ
                   | SYSCTL_SYSDIV_2_5);

    //
    // Setup peripherals
    //
    setup();

    //
    // Load the 8-bit unique node ID from the non-volatile user registers.
    //
    FlashUserGet(&ui32User0, &ui32User1);
    g_ui8ID = ui32User0 & 0xFF;

    //
    // Delay for radio startup.
    //
    SysCtlDelay(1000);

    //
    // Configure the radio.
    //
    nRFConfig(nRF_CFG_MASK_TX_DS | nRF_CFG_EN_CRC | nRF_CFG_PWR_UP);
    nRFFeatureSet(nRF_EN_DPL | nRF_EN_ACK_PAY);
    nRFDynPayloadEnable(nRF_DATA_PIPE_0);

    //
    // Enable interrupts from the radio
    //
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_0);
    MAP_IntEnable(INT_GPIOB_BLIZZARD);
    MAP_IntMasterEnable();
    
    //
    // Set the TX payload
    //
    nRFDataPut(&g_ui8ID, 1);
    nRFPayloadReuseEnable();


    //
    // Loop forever, requesting instructions at regular intervals.
    //
    while(1)
    {
        SysCtlDelay(100000000);

        //
        // Pulse the radio's chip enable.
        //
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);
        SysCtlDelay(500);
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0x00);
    }

}

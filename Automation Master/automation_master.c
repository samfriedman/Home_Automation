//*****************************************************************************
//
//--------------------------- Automation Master -------------------------------
//
// Command and Control Module for Home Automation.
//
// Copyright (c) 2014 Sam Friedman. All Rights Reserved.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "utils/ustdlib.h"

#include "utilities/nRF24L01.h"

void setup(void);
void ConfigureUART(void);
int CMD_help(int argc, char **argv);
int CMD_status(int argc, char **argv);
int CMD_verbose(int argc, char **argv);
int CMD_LED(int argc, char **argv);

bool g_bCMDReturn = false;
bool g_bVerbose = false;

uint32_t gui32SysClock;

//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
static char g_cInput[128];

uint8_t g_pui8AckData[5] = { 0xFF };

//*****************************************************************************
//
// A table of terminal commands, callback functions, and descriptions, as
// needed by the TivaWare command line processor.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",     CMD_help,      "    : Display list of commands" },
    {"status",   CMD_status,    "  : Read the radio's status register"},
    {"verbose",  CMD_verbose,   " : Toggle verbosity" },
    {"LED",      CMD_LED,       "     : \"LED state id\", where state = [on|off] and id = [0-4]"},
    { 0, 0, 0 }
};

//*****************************************************************************
//
// Takes two arguments, the first is either "on" or "off", the second is a
// number between 0 and 5 indicating to which slave to send the command. Sends
// a command to the indicated to slave to turn it's LED either on or off.
//
//*****************************************************************************
int
CMD_LED(int argc, char **argv)
{
    uint32_t ui32SlaveIndex;
    char* throwaway;
    if (argc > 2)
    {
        ui32SlaveIndex = ustrtoul(*(argv + 2), &throwaway, 10);
        if (!strcmp(*(argv + 1),"on"))
        {
            g_pui8AckData[ui32SlaveIndex] = 0xA1;
        } else if (!strcmp(*(argv + 1),"off"))
        {
            g_pui8AckData[ui32SlaveIndex] = 0xA2;
        } else {
            g_bCMDReturn = true;
            return CMDLINE_INVALID_ARG;
        }
        g_bCMDReturn = true;
        return 0;
    }
    g_bCMDReturn = true;
    return CMDLINE_TOO_FEW_ARGS;
}

//*****************************************************************************
//
// Print the contents of the radio's status register to the serial terminal.
//
//*****************************************************************************
int
CMD_status(int argc, char **argv)
{
    UARTprintf("%02x\n", nRFStatusGet());
    g_bCMDReturn = true;
    return(0);
}


//*****************************************************************************
//
// Write a help message to the serial terminal.
//
//*****************************************************************************
int
CMD_help(int argc, char **argv)
{
    tCmdLineEntry* psCommand = g_psCmdTable;
    while (psCommand->pcCmd)
    {
        UARTprintf(" ");
        UARTprintf(psCommand->pcCmd);
        UARTprintf(psCommand->pcHelp);
        UARTprintf("\n");
        psCommand++;
    }
    g_bCMDReturn = true;
    return(0);
}

//*****************************************************************************
//
// Toggle verbose output mode.
//
//*****************************************************************************
int
CMD_verbose(int argc, char **argv)
{
    if (g_bVerbose)
    {
        UARTprintf("Verbose mode off\n");
        g_bVerbose = false;
    } else {
        UARTprintf("Verbose mode on\n");
        g_bVerbose = true;
    }
    g_bCMDReturn = true;
    return(0);
}

//*****************************************************************************
//
// Flash the LED to signal an error and print an error message, MSG, to the
// serial terminal.
//
//*****************************************************************************
void
ErrorNotify(char* msg)
{
    int i;
    
    UARTprintf("ERROR: ");
    UARTprintf(msg);
    UARTprintf("\n");
    
    while (UARTBusy(UART0_BASE))
    {
        // spin
    }
    
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_4, 0x00);
    for (i = 0; i < 4; i++)
    {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, GPIO_PIN_5);
        SysCtlDelay(2000000);
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, 0x00);
        SysCtlDelay(2000000);
    }
}

//*****************************************************************************
//
// Handle interrupts from the radio.
//
//*****************************************************************************
void GPIOPortHIntHandler()
{
    uint8_t ui8RXData;
    int iSlaveIndex;
    
    //
    // Clear the interrupt.
    //
    GPIOIntClear(GPIO_PORTH_BASE, GPIO_INT_PIN_6);
    
    //
    // Finish SPI transmission, if any.
    //
    while(SSIBusy(SSI2_BASE))
    {
    }
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_7, GPIO_PIN_7);
    
    //
    // Clear the RX interrupt flag on the radio.
    //
    nRFClearInterrupt();
   
    //
    // Flush the RX FIFO.
    //
    //nRFFlushRX();
    nRFDataGet(&ui8RXData, 1);
    iSlaveIndex = ui8RXData & 0x0F;
    
    //ui8AckData = 0xA5;

    if (g_bVerbose)
    {
        UARTprintf("Request received from Node %d\n", iSlaveIndex);
    }
    
    if (g_pui8AckData[iSlaveIndex] != 0xFF) {
        nRFDataPutAck(0, g_pui8AckData + iSlaveIndex, 1);
        UARTprintf("Responding with %02x\n", g_pui8AckData[iSlaveIndex]);
        g_pui8AckData[iSlaveIndex] = 0xFF;
    }
}

int
main(void)
{
    int32_t i32CommandStatus;
    
    //
    // Set the system clock to run from the PLL at 120 MHz
    //
    gui32SysClock = SysCtlClockFreqSet(SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_25MHZ
                   | SYSCTL_CFG_VCO_480, 120000000);
    
    setup();
    
    //
    // Enable data receive interrupt and enable radio for RX mode
    // (Mask data sent and max re-transmit interrupts)
    //
    nRFConfig(/*nRF_CFG_MASK_TX_DS | nRF_CFG_MASK_MAX_RT | */nRF_CFG_EN_CRC
                | nRF_CFG_PWR_UP | nRF_CFG_PRIM_RX);
    nRFFeatureSet(nRF_EN_DPL | nRF_EN_ACK_PAY);
    nRFDynPayloadEnable(nRF_DATA_PIPE_0);
    
    //
    // Enable the GPIO interrupt for the radio IRQ.
    //
    GPIOIntEnable(GPIO_PORTH_BASE, GPIO_INT_PIN_6);
    MAP_IntEnable(INT_GPIOH_SNOWFLAKE);
    MAP_IntMasterEnable();
    
    //
    // Set up UART and initialize command line
    //
    ConfigureUART();
    
    UARTprintf("\nHome Automation Console\n");
    UARTprintf("Type \"help\" for a list of commands\n");
    UARTprintf("> ");
    
    //
    // Enable the Radio
    //
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_7, GPIO_PIN_7);
    uint8_t ui8AckData = 0xA5;
    
    nRFDataPutAck(0, &ui8AckData, 1);
    
    while(1)
    {
        
        
        //
        // Process commands from the UART.
        //
        if (UARTPeek('\r') != -1)
        {
            g_bCMDReturn = false;
            
            UARTgets(g_cInput,sizeof(g_cInput));

            //
            // Pass the line from the user to the command processor.
            // It will be parsed and valid commands executed.
            //
            i32CommandStatus = CmdLineProcess(g_cInput);

            //
            // Handle the case of bad command.
            //
            if(i32CommandStatus == CMDLINE_BAD_CMD)
            {
                ErrorNotify("Bad Command");
            }
            
            else if (i32CommandStatus == CMDLINE_TOO_FEW_ARGS)
            {
                ErrorNotify("Too few arguments!");
            }
            
            else if (i32CommandStatus == CMDLINE_INVALID_ARG)
            {
                ErrorNotify("Invalid argument!");
            }

            //
            // Handle the case of too many arguments.
            //
            else if(i32CommandStatus == CMDLINE_TOO_MANY_ARGS)
            {
                UARTprintf("Too many arguments for command processor!\n");
            }
            
            else if(!g_bCMDReturn)
            {
                while(!g_bCMDReturn);
            }
            
            UARTprintf("> ");
        }
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Setup peripherals, clock gating, and pin-muxing.
//
//*****************************************************************************
void
setup()
{ 
    //
    // Enable peripherals
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    
    //
    // Setup interrupt on radio IRQ assertion.
    //
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_6);
    MAP_GPIOIntTypeSet(GPIO_PORTH_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
    
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_7);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_7);
    
    //
    // Configure pins for SSI2
    //
    MAP_GPIOPinConfigure(GPIO_PG5_SSI2XDAT0);
    MAP_GPIOPinConfigure(GPIO_PG4_SSI2XDAT1);
    MAP_GPIOPinConfigure(GPIO_PG7_SSI2CLK);
    MAP_GPIOPinTypeSSI(GPIO_PORTG_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_7, GPIO_PIN_7);
    
    //
    // Configure pins for LED output
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_5);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_4);
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, 0x00);
    GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_4, 0x00);
    
    GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_7, 0x00);
    
    //
    // Confiure SSI2 for SPI Mode 0 at 8Mbps, 8 bit transfers.
    //
    MAP_SSIConfigSetExpClk(SSI2_BASE, gui32SysClock, SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER, 8000000, 8);
    MAP_SSIEnable(SSI2_BASE);
}

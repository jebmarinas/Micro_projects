
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"#include "inc/tm4c123gh6pm.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_adc.h"
#include "inc/hw_types.h"
#include "inc/hw_udma.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "utils/uartstdio.h"

#define ADC_SAMPLE_BUF_SIZE     1

#if defined(ewarm)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

//Structs
enum BUFFER_STATUS
{
    EMPTY,
    FULL
};

typedef struct{
    uint16_t adcBuffer[ADC_SAMPLE_BUF_SIZE];
    uint32_t AVG;
    uint32_t C;
    uint32_t F;
}Temp_t;

typedef enum{
    WAIT,
    RED, BLUE, GREEN,
    red, blue, green,
    TEMP,
    temp,
    STAT

}State_enum;

typedef struct{

    unsigned char input;

}UART_t;
///////////////////////////////////////////
//Variables
volatile static enum BUFFER_STATUS BufferStatus;
volatile Temp_t temp_t;
volatile uint32_t Count;
///////////////////////////////////////////
//Prototype
void CalculateTemperatureAvg(void);
void ConfigureUART(void);
void ConfigureADC(void);
void ConfigureUDMA(void);
void check_status(uint32_t status, const char* string);
void send_status(const char* string, bool status);
///////////////////////////////////////////
#define GREEN_LED GPIO_PIN_3
#define BLUE_LED  GPIO_PIN_2
#define RED_LED   GPIO_PIN_1
///////////////////////////////////////////
int main(void){

    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_INT |   SYSCTL_XTAL_16MHZ);

    volatile unsigned char status_of_leds;

    volatile State_enum state, nextState = WAIT;

    volatile UART_t uart_t;
    BufferStatus = EMPTY;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);

    //Config LEDs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    ConfigureUART();
    ConfigureUDMA();
    ConfigureADC();

    IntMasterEnable();

    while(1)
    {
        state = nextState;

        switch(state){

                case WAIT:

                    while(!UARTCharsAvail(UART0_BASE)){};

                    uart_t.input = UARTCharGet(UART0_BASE);

                         if(uart_t.input == 'R') nextState = RED;
                    else if(uart_t.input == 'B') nextState = BLUE;
                    else if(uart_t.input == 'G') nextState = GREEN;
                    else if(uart_t.input == 'r') nextState = red;
                    else if(uart_t.input == 'b') nextState = blue;
                    else if(uart_t.input == 'g') nextState = green;
                    else if(uart_t.input == 'T') nextState = TEMP;
                    else if(uart_t.input == 't') nextState = temp;
                    else if(uart_t.input == 'S') nextState = STAT;
                    else                         nextState = WAIT;
                break;

                case RED:
                    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, RED_LED);
                    nextState = WAIT;
                    break;

                case red:
                    GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, 0);
                    nextState = WAIT;
                    break;
                case BLUE:
                    GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, BLUE_LED);
                    nextState = WAIT;
                    break;

               case blue:
                    GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, 0);
                    nextState = WAIT;
                    break;

               case GREEN:
                    GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, GREEN_LED);
                    nextState = WAIT;
                    break;

               case green:
                    GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, 0);
                    nextState = WAIT;
                    break;

               case TEMP:
                    CalculateTemperatureAvg();
                    UARTprintf("C %3d\t", temp_t.C);
                    UARTprintf("\n");
                    nextState = WAIT;
                    break;


               case temp:
                    CalculateTemperatureAvg();
                    UARTprintf("F: %3d\t",temp_t.F);
                    UARTprintf("\n");
                    nextState = WAIT;
                    break;


               case STAT:
                    status_of_leds = GPIOPinRead(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED);
                    check_status((status_of_leds&RED_LED)   >> 1, "Red:   ");
                    check_status((status_of_leds&BLUE_LED)  >> 2, "Blue:  ");
                    check_status((status_of_leds&GREEN_LED) >> 3, "Green: ");
                    nextState = WAIT;
                    break;

               default:
                    nextState = WAIT;
                    break;
        }

    }
}


void check_status(uint32_t status, const char* string){
    if(status)
       send_status(string, 1);
    else
       send_status(string, 0);
}

void send_status (const char* string, bool status){

    UARTprintf(string);

    if(status)
        UARTprintf("ON");
    else
       UARTprintf("OFF");

    UARTprintf("\n");
}

void CalculateTemperatureAvg(void){

    ADCProcessorTrigger(ADC0_BASE, 0);

    while(BufferStatus == EMPTY){};

                temp_t.AVG = 0;

                for(Count = 0; Count < ADC_SAMPLE_BUF_SIZE; Count++)
                {
                    temp_t.AVG += temp_t.adcBuffer[Count];
                    temp_t.adcBuffer[Count] = 0;
                }

                BufferStatus = EMPTY;

                uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                                       UDMA_MODE_BASIC,
                                       (void *)(ADC0_BASE + ADC_O_SSFIFO0),
                                       &temp_t.adcBuffer, ADC_SAMPLE_BUF_SIZE);

                uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT);

                temp_t.AVG = ((temp_t.AVG +
                                      (ADC_SAMPLE_BUF_SIZE / 2)) /
                                       ADC_SAMPLE_BUF_SIZE);

                temp_t.C = (1475 - ((2475 * temp_t.AVG)) / 4096)/10;
                temp_t.F = ((temp_t.C * 9) + 160) / 5;
}

void ConfigureUART(void){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


    UARTStdioConfig(0, 115200, SysCtlClockGet());
}


void ConfigureADC(void){

        IntDisable(INT_ADC0SS0);

        ADCIntDisable(ADC0_BASE, 0);


        ADCSequenceDisable(ADC0_BASE, 0);


        ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);


        ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS | ADC_CTL_END |
                                 ADC_CTL_IE);


        ADCSequenceEnable(ADC0_BASE, 0);

        ADCIntClear(ADC0_BASE, 0);

        ADCSequenceDMAEnable(ADC0_BASE, 0);

        ADCIntEnable(ADC0_BASE, 0);

        IntEnable(INT_ADC0SS0);


}


void ConfigureUDMA(void){

       uDMAEnable();


       uDMAControlBaseSet(pui8ControlTable);

       uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0,
                                   UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY |
                                   UDMA_ATTR_REQMASK);


       uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_SIZE_16 |
                             UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_64);


       uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                              UDMA_MODE_BASIC,
                              (void *)(ADC0_BASE + ADC_O_SSFIFO0),
                              &temp_t.adcBuffer, ADC_SAMPLE_BUF_SIZE);


       uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);


       uDMAChannelEnable(UDMA_CHANNEL_ADC0);
}

void ADCSeq0Handler(void){

    ADCIntClear(ADC0_BASE, 0);

    if ((uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT) == UDMA_MODE_STOP))
    {
        BufferStatus = FULL;

    }

}


#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include <string.h>


#ifdef DEBUG
void__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// Globals
uint32_t ui32Period;
char     buffer[4];

uint32_t ui32ADC0Value[4];
volatile uint32_t ui32TempAvg;
volatile uint32_t ui32TempValueC;
volatile uint32_t ui32TempValueF;

void ConfigurePORT(void);

// Timer 1 ISR
void Timer1IntHandler(void)
{
     // Clear the timer interrupt
     TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

     ADCIntClear(ADC0_BASE, 2);
     ADCProcessorTrigger(ADC0_BASE, 2);

     ADCSequenceDataGet(ADC0_BASE, 2, ui32ADC0Value);

     ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;
     ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;
     ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;
     UARTprintf("C %3d\t",ui32TempValueC );
     UARTprintf("F %3d\t",ui32TempValueF );
     UARTprintf("\n");
}



int main(void) {

    // Configure Clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Configure peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // Enabling Timer 1

    ConfigurePORT();

    // Configure ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCHardwareOversampleConfigure(ADC0_BASE, 32);

    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0); // Changed to sequencer #2

    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_TS);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_TS);

    ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    ADCSequenceEnable(ADC0_BASE, 2);

    //register the interrupt handler for PF0
   // GPIOIntRegister(GPIO_PORTF_BASE, GPIOF0IntHandler);
    //SW2 goes low when pressed
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);

    // Configure Timer 1 module
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    ui32Period = SysCtlClockGet()/2;   // Period of 0.5s 2Hz
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);
    IntEnable(INT_TIMER1A);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Configure pins for UART
    //GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);

    // Enable interrupts
    IntMasterEnable();
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0);
    TimerEnable(TIMER1_BASE, TIMER_A);
    ADCSequenceEnable(ADC0_BASE, 2);

    // Initial message to terminal display
    UARTprintf("Temperature:\n");
    while (1){};

}

void ConfigurePORT(void)
{
    //Port configuration (LEDS)
    //Enable GPIOF port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //set LEDS connected to pins as outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //Unlock Pin F0 to use an interrupt on SW2
    SYSCTL_RCGC2_R |= 0x00000020;     // activate clock for Port F
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F
    GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
    // only PF0 needs to be unlocked, other bits can't be locked
    GPIO_PORTF_AMSEL_R = 0x00;        // disable analog on PF
    GPIO_PORTF_PCTL_R = 0x00000000;   // PCTL GPIO on PF4-0
    GPIO_PORTF_DIR_R = 0x0E;          // PF4,PF0 in, PF3-1 out
    GPIO_PORTF_AFSEL_R = 0x00;        // disable alt funct on PF7-0
    GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
    GPIO_PORTF_DEN_R = 0x1F;          // enable digital I/O on PF4-0
}

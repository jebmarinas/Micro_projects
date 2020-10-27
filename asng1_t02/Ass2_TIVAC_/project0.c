#include "inc/tm4c123gh6pm.h"
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
#include "driverlib/i2c.h"


void ConfigureUART(void);
void initI2C0(void);
void I2C0_Read16(uint8_t, uint8_t ,uint16_t*);
//void I2C0_send(uint8_t, uint8_t);



int main(void)
{
    uint16_t *var;
    initI2C0();
    //I2C0_send(0x40,0xff);
    I2C0_Read16(0x40,0xff,var);
    ConfigureUART();
    UARTprintf("F: %3d\t\n",*var);




}
    void initI2C0(void) // initialized my I2C 0
    {
            SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
            SysCtlDelay(3);

            SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
            SysCtlDelay(3);

            GPIOPinConfigure(GPIO_PB2_I2C0SCL);
            GPIOPinConfigure(GPIO_PB3_I2C0SDA);

            GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
            GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

            I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);// turn I2c Automatically/ and configures it
    }
   /* void I2C0_send(uint8_t slave_addr,uint8_t num_of_args)
    {
        I2CMasterSlaveAddrSet(I2C0_BASE,slave_addr,false);
        va_list vargs;
        va_start(vargs,num_of_args);
        I2CMasterDataPut(I2C0_BASE,va_arg(vargs,uint32_t));
        I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
        while(I2CMasterBusy(I2C0_BASE));
        va_end(vargs);
        I2CMasterDataPut(I2C0_BASE,va_arg(vargs,uint32_t));
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        while(I2CMasterBusy(I2C0_BASE));
       va_end(vargs);

    }*/
    void I2C0_Read16(uint8_t slave_addr, uint8_t pointer_reg, uint16_t* RxData )
    {
        uint8_t data;
        I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);
        I2CMasterDataPut(I2C0_BASE, pointer_reg);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C0_BASE));
        I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(I2C0_BASE));
        //MSB first
        data = I2CMasterDataGet(I2C0_BASE);
        *RxData = (uint16_t)(data << 8);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(I2C0_BASE));
        //LSB later
        data = I2CMasterDataGet(I2C0_BASE);
        *RxData |= (uint16_t)(data);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        while(I2CMasterBusy(I2C0_BASE));
    }


    void ConfigureUART(void){

        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


        SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

        GPIOPinConfigure(GPIO_PA0_U0RX);
        GPIOPinConfigure(GPIO_PA1_U0TX);
        GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


        UARTStdioConfig(0, 115200, SysCtlClockGet());
    }







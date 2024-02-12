/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* RTC Register Reading                                              */
/*-------------------------------------------------------------------*/

#include <msp430.h>

/*-------------------------------------------------------------------*/
/* Variable Definitions for Reading RTC Registers                    */
/*-------------------------------------------------------------------*/

// RTC Global Variables

unsigned int t1 = 0;
static const int index = 0;
static const char address[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D};
char settings[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};     // Readings from Registers in RTC

/*-------------------------------------------------------------------*/
/* I2C Initialization for RTC Communication                          */
/*-------------------------------------------------------------------*/
void init_RTC(void) {

    UCB0CTLW0 |= UCMODE_3;                  // Put into I2C mode
    UCB0BRW |= 10;                          // Prescalar = 10
    UCB0CTLW0 |= UCMST;                     // Master mode
    UCB0CTLW0 &= ~UCTR;                     // Read/RX mode
    UCB0I2CSA = 0x68;                       // Slave Address = 0x68
    UCB0TBCNT = 1;                          // Byte count (1 byte per register in RTC)
    UCB0CTLW1|= UCASTP_2;                   // Auto stop mode

    P1SEL1 &= ~BIT3;                        // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                        // P1.2 = SDA
    P1SEL0 |= BIT2;

}

/*-------------------------------------------------------------------*/
/* Read from RTC Registers                                           */
/*-------------------------------------------------------------------*/
void get_RTC(void) {

    int i;

    for(i = 0; i < (sizeof(address) - index + 1); i++) {        // Iterate based on number of byte, offset by the starting address within the RTC

        if(i == 0) {                                            // First byte should be a transmission of the starting address in RTC

            UCB0CTLW0 |= UCTR;
            UCB0CTLW0 |= UCTXSTT;

            while((UCB0IFG & UCSTPIFG) == 0) {}
            UCB0IFG &= ~UCSTPIFG;


        } else {                                                // All following bytes should be received from RTC registers

            UCB0CTLW0 &= ~UCTR;
            UCB0CTLW0 |= UCTXSTT;

            while((UCB0IFG & UCSTPIFG) == 0) {}
            UCB0IFG &= ~UCSTPIFG;

            t1 = t1 + 1;                                        // Increment byte counter

        }

    }

    UCB0IFG &= ~UCRXIFG0;                   // Clear flag to allow I2C interrupt

}


/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    UCB0CTLW0 |= UCSWRST;           // Software reset for I2C

    init_RTC();                     // Initialize I2C settings for RTC communication

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O
    UCB0CTLW0 &= ~UCSWRST;          // Take out of software reset for I2C

    // Enable interrupts
    UCB0IE |= UCTXIE0 | UCRXIE0;    // Local interrupt enable for I2C TX0/RX0

    __enable_interrupt();           // Global IRQ enable

    int i;

    while(1){
        get_RTC();                      // Get current data stored in RTC registers
        for(i = 0; i < 100; i++) {}
    }


    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    switch(UCB0IV){
        case 0x16:                                      // Receiving
            settings[t1] = UCB0RXBUF;                   // Place received byte from buffer into packet 
            break;
        case 0x18:                                      // Transmitting
            UCB0TXBUF = address[index];                 // Transmit starting address within RTC
            break;
    }

}

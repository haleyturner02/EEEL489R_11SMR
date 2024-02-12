/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* RTC Initialization for Writing to Registers                       */
/*-------------------------------------------------------------------*/

#include <msp430.h>

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/

// RTC Global Variables

unsigned int t1 = 0;                        // Bit counter for packet
char setup[] = {0x00, 0x00, 0x09, 0x08, 0x00, 0x11, 0x02, 0x24, 0x00, 0x00, 0x00, 0x00, 0x15, 0x80, 0x80};
// Packet: {Starting Register Address, Seconds, Minutes, Hours, DOW, Day, Month, Year, A1, A1, A1, A1, A2, A2, A2}

/*-------------------------------------------------------------------*/
/* I2C Initialization for RTC Communication                          */
/*-------------------------------------------------------------------*/
void init_RTC(void) {

    UCB0CTLW0 |= UCMODE_3;                  // Put into I2C mode
    UCB0BRW |= 10;                          // Prescalar = 10
    UCB0CTLW0 |= UCMST;                     // Master mode
    UCB0CTLW0 &= ~UCTR;                     // Read/RX mode
    UCB0I2CSA = 0x68;                       // Slave Address = 0x68
    UCB0TBCNT = sizeof(setup);              // Byte count (1 byte per register in RTC)
    UCB0CTLW1|= UCASTP_2;                   // Auto stop mode

    P1SEL1 &= ~BIT3;                        // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                        // P1.2 = SDA
    P1SEL0 |= BIT2;

}

/*-------------------------------------------------------------------*/
/* RTC Initialization for Alarm                                      */
/*-------------------------------------------------------------------*/
void setup_RTC(void) {

    UCB0CTLW0 |= UCTR;                              // Set to transmit to RTC
    UCB0CTLW0 |= UCTXSTT;                           // Send start message

    while((UCB0IFG & UCSTPIFG) == 0) {}             // Wait for Auto-Stop to end transmission
    UCB0IFG &= ~UCSTPIFG;                           // Clear TX Flag

}


/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    UCB0CTLW0 |= UCSWRST;           // Software reset for I2C

    init_RTC();

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O
    UCB0CTLW0 &= ~UCSWRST;          // Take out of software reset for I2C

    // Enable interrupts
    UCB0IE |= UCTXIE0;              // Local interrupt enable for I2C TX0

    __enable_interrupt();           // Global IRQ enable

    int i;

    setup_RTC();                    // Setup RTC Registers
    UCB0IE &= ~UCTXIE0;             // Disable I2C TX interrupt

    while(1){}

    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    UCB0TXBUF = setup[t1];                      // Place next byte into I2C TX buffer
    t1 = t1 + 1;                                // Increment to next byte in packet
    if(t1 == sizeof(setup)) {                   // Determine if entire packet has been sent
        t1 = 0;                                 // Reset byte counter
        UCB0IFG &= ~UCTXIFG0;                   // Clear flag to allow I2C interrupt
    }

}

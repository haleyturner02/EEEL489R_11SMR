/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* RTC Initialization                                                */
/*-------------------------------------------------------------------*/

#include <msp430.h>

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/

// RTC Global Variables

unsigned int t1 = 0;
char Data_In;
char setup[] = {0x0B, 0x01};                // Setup: 0x0B -> 0x15, 0x0C -> 0x80, 0x0D -> 0x80
char alarm;


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

    UCB0CTLW0 |= UCTR;
    UCB0CTLW0 |= UCTXSTT;

}

void get_setup(void) {

    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT;

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
    UCB0IE |= UCTXIE0 | UCRXIE0;    // Local interrupt enable for I2C TX0/RX0

    __enable_interrupt();           // Global IRQ enable

    int i;

    setup_RTC();
    for(i = 0; i < 100; i++) {}
    get_setup();
    for(i = 0; i < 100; i++) {}

    while(1){}


    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    switch(UCB0IV){
        case 0x16:                                      // Receiving
            Data_In = UCB0RXBUF;                        // Retrieve byte from buffer
            t1 = t1 + 1;
            if(t1 == sizeof(setup)) {
                t1 = 0;
                UCB0IFG &= ~UCRXIFG0;                   // Clear flag to allow I2C interrupt
            } else if (t1 == 1) {
                alarm = Data_In;
            }
            break;
        case 0x18:
            UCB0TXBUF = setup[t1];
            t1 = t1 + 1;
            if(t1 == sizeof(setup)) {
                t1 = 0;
                UCB0IFG &= ~UCTXIFG0;                   // Clear flag to allow I2C interrupt
            }
            break;
    }

}

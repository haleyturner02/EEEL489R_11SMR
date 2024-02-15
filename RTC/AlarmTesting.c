#include <msp430.h>

/*-------------------------------------------------------------------*/
/* Variable Definitions for Reading RTC Registers                    */
/*-------------------------------------------------------------------*/

// RTC Global Variables

volatile unsigned int t1 = 0;
volatile unsigned int set = 0;
unsigned int count = 0;
char reset[] = {0x0F, 0x80};
char time[] = {0x2A, 0x0F};

/*-------------------------------------------------------------------*/
/* I2C Initialization for RTC Communication                          */
/*-------------------------------------------------------------------*/
void init_RTC(void) {

    UCB0CTLW0 |= UCMODE_3;                  // Put into I2C mode
    UCB0BRW |= 10;                          // Prescalar = 10
    UCB0CTLW0 |= UCMST;                     // Master mode
    UCB0CTLW0 &= ~UCTR;                     // Read/RX mode
    UCB0I2CSA = 0x68;                       // Slave Address = 0x68
    UCB0TBCNT = sizeof(reset);              // Byte count (1 byte per register in RTC)
    UCB0CTLW1|= UCASTP_2;                   // Auto stop mode

    P1SEL1 &= ~BIT3;                        // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                        // P1.2 = SDA
    P1SEL0 |= BIT2;

}

/*-------------------------------------------------------------------*/
/* Update time based on Alarm event                                  */
/*-------------------------------------------------------------------*/
void updateTime(void) {

    // Set Hours
    if(time[1] == 0x17) {
        time[1] = 0x00;
    } else if(time[0] == 0x3B) {
        time[1] = time[1] + 0x01;
    }

    // Set Minutes
    if(time[0] == 0x3B) {                   // Minutes = 59, hour needs to change as well
        time[0] = 0x00;
    } else {
        time[0] = time[0] + 0x01;
    }

}

/*-------------------------------------------------------------------*/
/* Clear RTC Alarm 2 Flag                                            */
/*-------------------------------------------------------------------*/
void clearAlarm(void) {

    int i;

    UCB0CTLW0 |= UCTR;
    UCB0CTLW0 |= UCTXSTT;

    while((UCB0IFG & UCSTPIFG) == 0) {}
    UCB0IFG &= ~UCSTPIFG;

    for(i = 0; i < 1000; i++) {}

    P3IE |= BIT0;                   // Enable IRQ for SQW
    set = 0;

}


/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
    UCB0CTLW0 |= UCSWRST;           // Software reset for I2C

    // I/O Defintiions
    P1DIR |= BIT0;                  // Set P1.0 (LED1) as output
    P1OUT &= ~BIT0;                 // Start LED1 off

    P3DIR &= ~BIT0;                 // Set P3.0 (SQW) as input
    P3REN |= BIT0;                  // Enable pull up/down resistors
    P3OUT |= BIT0;                  // Set as pull up resistor
    P3IES |= BIT0;                  // High to Low Sensitivity for SQW

    init_RTC();                     // Initialize I2C settings for RTC communication

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O
    UCB0CTLW0 &= ~UCSWRST;          // Take out of software reset for I2C

    // Enable interrupts
    UCB0IE |= UCTXIE0;              // Local interrupt enable for I2C TX0

    P3IE |= BIT0;                   // Enable IRQ for SQW
    P3IFG &= ~BIT0;                 // Clear flags for SQW

    __enable_interrupt();           // Global IRQ enable

    clearAlarm();
    while(1){

        if(set == 1) {
            updateTime();
            clearAlarm();
        }

    }

    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    UCB0TXBUF = reset[t1];                      // Place next byte into I2C TX buffer
    t1 = t1 + 1;                                // Increment to next byte in packet
    if(t1 == sizeof(reset)) {                   // Determine if entire packet has been sent
        t1 = 0;                                 // Reset byte counter
        UCB0IFG &= ~UCTXIFG0;                   // Clear flag to allow I2C interrupt
    }

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC Alarm                              */
/*-------------------------------------------------------------------*/
#pragma vector = PORT3_VECTOR
__interrupt void PORT3_ISR(void){

    P1OUT ^= BIT0;
    set = 1;
    if(count < 15) {
        count = count + 1;
    } else {
        count = 0;
        // Call for new measurement
    }

    P3IE &= ~BIT0;                  // Temporarily disable IRQ for SQW
    P3IFG &= ~BIT0;                 // Clear flags for SW1


}

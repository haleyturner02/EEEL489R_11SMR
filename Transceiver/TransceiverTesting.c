/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Transceiver Setup and Testing                                     */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

volatile unsigned int received_value = 0;                                                  // Receiving state indicatorvolatile unsigned int received_value = 0;                                                  // Indicator for UART receive from BLE to MSP
volatile char Data_In;                                                                     // Stores received byte

/*-------------------------------------------------------------------*/
/* UART1 Initialization for Transceiver Communication                */
/*-------------------------------------------------------------------*/
void init_Transceiver() {
    // Settings to acheive baud rate of 115200

    UCA1CTLW0 |= UCSSEL__SMCLK;         // Use SMCLK
    UCA1BRW = 8;                        // Prescaler = 8
    UCA1MCTLW = 0xD600;                 // Set modulation and low frequency

    P4SEL1 &= ~BIT2;                    // Set P4.2 to UART A1 RX
    P4SEL0 |= BIT2;

    P4SEL1 &= ~BIT3;                    // Set P4.3 UART A1 TX
    P4SEL0 |= BIT3;

}

/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    UCA1CTLW0 |= UCSWRST;                               // Put into software reset

    /* I/O Defintiions */
    P1DIR |= BIT0;                                      // Set P1.0 (LED1) as output
    P1OUT &= ~BIT0;                                     // Start LED1 off

    init_Transceiver();

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCA1CTLW0 &= ~UCSWRST;                              // Take out of software reset for I2C

    __enable_interrupt();                               // Global IRQ enable

    int i;

    while(1){

        // Receiving byte from transceiver to microcontroller
        UCA1IE |= UCRXIE;                               // Enable UART A1 RX
        while(received_value == 0) {}                   // Wait for value to be received
        received_value = 0;                             // Clear received value indicator
        UCA1IE &= ~UCRXIE;                              // Disable UART A1 RX

        // Transmitting byte from microcontroller to transceiver
        UCA1TXBUF = Data_In;                             // Change value to desired byte
        for(i = 0; i < 100; i++){}                      // Delay for UART transmission


    }

    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: UART1 Communication                    */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_A1_VECTOR
__interrupt void EUSCI_A1_RX_ISR(void) {

    Data_In = UCA1RXBUF;                                            // Receive byte from RX buffer (note: use ASCII character table)

    received_value = 1;                                             // Set indicator to alert that value has been received

    UCA1IFG &= ~UCRXIFG;                                            // Clear flags for UART A1 RX

}

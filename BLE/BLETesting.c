#include <msp430.h> 

volatile unsigned int u1;
volatile unsigned int receiving = 0;                                                        // Receiving state indicator
volatile unsigned int received_value = 0;                                                  // Indicator for UART receive from BLE to MSP
volatile unsigned char received_data [] = {0x00, 0x00, 0x00};
int prev_measurements[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Each measurement is formatted as <XXX>, where < and > are delimiters

/*-------------------------------------------------------------------*/
/* UART1 Initialization for BLE Communication                        */
/*-------------------------------------------------------------------*/
void init_BLE() {
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
/* Function for converting and storing received measurement          */
/*-------------------------------------------------------------------*/
void store_measurement(void) {

   unsigned int i, m, s;                             // Variables for mapping char data to int measurement (i=0 -> m hundreds, i=1 -> m tens, i=2 -> m ones)
   m = 0;                                   // Start measurement value at 0

   for(i = 0; i < 3; i++) {                 // Convert all 3 characters received and add to measurement sum
       switch(i) {                          // Getting scaling factor for char data based on position in received_data
           case 0:                          // First received character corresponds to hundreds place
               s = 100;
               break;
           case 1:                          // Second received character corresponds to tens place
               s = 10;
               break;
           case 2:                          // Third received character corresponds to ones place
               s = 1;
               break;
           default:
               break;
       }

       switch(received_data[i]) {           // Convert ASCII character code to integer and add to measurement sum
           case 0x30:
              m = m + 0;
              break;
           case 0x31:
               m = m + 1*s;
               break;
           case 0x32:
               m = m + 2*s;
               break;
           case 0x33:
               m = m + 3*s;
               break;
           case 0x34:
               m = m + 4*s;
               break;
           case 0x35:
               m = m + 5*s;
               break;
           case 0x36:
               m = m + 6*s;
               break;
           case 0x37:
               m = m + 7*s;
               break;
           case 0x38:
               m = m + 8*s;
               break;
           case 0x39:
               m = m + 9*s;
               break;
           default:
               break;
       }

   }

   for(i = 11; i > 0; i--) {                                    // Shift previous measurement array
       prev_measurements[i] = prev_measurements[i-1];
   }

   prev_measurements[0] = m;                                    // Store newest measurement

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

    init_BLE();

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCA1CTLW0 &= ~UCSWRST;                              // Take out of software reset for I2C

    __enable_interrupt();                               // Global IRQ enable

    while(1){

        UCA1IE |= UCRXIE;                               // Enable UART A1 RX
        while(received_value == 0) {}                   // Wait for value to be received
        received_value = 0;                             // Clear received value indicator
        UCA1IE &= ~UCRXIE;                              // Disable UART A1 RX

    }

    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: BLE UART1 Communication                */
/*-------------------------------------------------------------------*/

#pragma vector = EUSCI_A1_VECTOR
__interrupt void EUSCI_A1_RX_ISR(void) {

    if(receiving == 0 && UCA1RXBUF == 0x3C) {                       // State 0: Receiving = 0 -> wait for '<' to be received to indicate start of data
        receiving = 1;
    } else if(receiving == 1) {                                     // State 1: Receiving = 1 -> receive hundreds place value of measurement
        received_data[0] = UCA1RXBUF;
        receiving = 2;
    } else if (receiving == 2) {                                    // State 2: Receiving = 2 -> receive tens place value of measurement
        received_data[1] = UCA1RXBUF;
        receiving = 3;
    } else if (receiving == 3) {                                    // State 3: Receiving = 3 -> receive ones place value of measurement
        received_data[2] = UCA1RXBUF;
        receiving = 4;
    } else if (receiving == 4 && UCA1RXBUF == 0x3E) {               // State 4: Receiving = 4 -> wait for '>' to be received to indicate end of data
        store_measurement();
        receiving = 0;
    }

    received_value = 1;                                             // Set indicator to alert that value has been received

    UCA1IFG &= ~UCRXIFG;                                            // Clear flags for UART A1 RX

}

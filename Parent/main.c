/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/
unsigned int u1 = 0;                                    // Counter for UART1

// Measurement Global Variables
char measurement1[] = "0000";                           // Newest measurement
char measurement2[] = "0000";                           // Measurement from 15 minutes prior
char measurement3[] = "0000";                           // Measurement from 30 minutes prior
char measurement4[] = "0000";                           // Measurement from 45 minutes prior

// RTC Global Variables
unsigned int alarm = 0;                                                                     // Indicates if alarm is being setup
unsigned int t1 = 0;                                                                        // Indicator for which RTC register is being read from
char Data_In;                                                                               // Temporarily stores received byte from RTC
char time[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};     // Packet to contain time (day, month, year, hour, minute, second)
// Packet Format: {sec1, sec2, min1, min2, hour1, hour2, day1, day2, month1, month2, year}
// Packet Example: February 8th, 2024 at 2:43:13 -> {0x03, 0x01, 0x03, 0x04, 0x02, 0x0, 0x08, 0x00, 0x02, 0x00, 0x04, 0x02}


/*-------------------------------------------------------------------*/
/* UART1 Initialization for BLE Communication                        */
/*-------------------------------------------------------------------*/
void init_BLE() {
    // Settings to acheive baud rate of 115200
    UCA1CTLW0 |= UCSSEL_3;      // Use SMCLK
    UCA1BRW = 8;                // Prescaler = 8
    //UCBRS1 = 0xD6;            // Modulation = 0xD6

    P4SEL1 &= ~BIT2;            // Set P4.2 to UART1 RX
    P4SEL0 |= BIT2;

    P4SEL1 &= ~BIT3;            // Set P4.3 to UART1 TX
    P4SEL0 |= BIT3;
}

/*-------------------------------------------------------------------*/
/* I2C Initialization for RTC Communication                          */
/*-------------------------------------------------------------------*/
void init_RTC(void) {

    UCB0CTLW0 |= UCMODE_3;      // Put into I2C mode
    UCB0BRW |= 10;              // Prescalar = 10
    UCB0CTLW0 |= UCMST;         // Master mode
    UCB0CTLW0 &= ~UCTR;         // Read/RX mode
    UCB0I2CSA = 0x68;           // Slave Address = 0x68
    UCB0TBCNT = 1;              // Byte count (1 byte per register in RTC)
    UCB0CTLW1|= UCASTP_2;       // Auto stop mode

    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2;

}

/*-------------------------------------------------------------------*/
/* Alarm Setup Function for creating 15 minute alarm on RTC          */
/*-------------------------------------------------------------------*/
void setup_alarm() {

    int i;
    alarm = 1;

    UCB0CTLW0 |= UCTR;
    UCB0CTLW0 |= UCTXSTT;
    for(i = 0; i < 100; i++) {}

    alarm = 0;

}

/*-------------------------------------------------------------------*/
/* Get Time Function for reading time/date from RTC registers        */
/*-------------------------------------------------------------------*/
void getTime(void) {

    UCB0CTLW0 |= UCTR;
    UCB0CTLW0 |= UCTXSTT;

    while((UCB0IFG & UCSTPIFG) == 0) {}
    UCB0IFG &= ~UCSTPIFG;

    UCB0CTLW0 &= ~UCTR;
    UCB0CTLW0 |= UCTXSTT;

    while((UCB0IFG & UCSTPIFG) == 0) {}
    UCB0IFG &= ~UCSTPIFG;
}

/*-------------------------------------------------------------------*/
/* Set Time Function for storing time and data from RTC in packet    */
/*-------------------------------------------------------------------*/
void setTime(void) {

    switch(t1) {
        case 0:
           time[0] = Data_In & 0x0F;                    // Get lower nibble for sec1 (one's place of seconds)
           time[1] = (Data_In & 0x70) >> 4;             // Get upper nibble (not including MSB) for sec2 (ten's place of seconds)
           break;
        case 1:
            time[2] = Data_In & 0x0F;                   // Get lower nibble for min1 (one's place of minutes)
            time[3] = (Data_In & 0x70) >> 4;            // Get upper nibble (not including MSB) for min2 (ten's place of minutes)
            break;
        case 2:
            time[4] = Data_In & 0x0F;                   // Get lower nibble for hour1 (one's place of hour)
            time[5] = (Data_In & 0x08) >> 4;            // Get bit 5 for hour2 (ten's place of hour)
            break;
        case 4:
            time[6] = Data_In & 0x0F;                   // Get lower nibble for day1 (one's place of day)
            time[7] = (Data_In & 0x30) > 4;             // Get bits 5 & 6 for day2 (ten's place of day)
            break;
        case 5:
            time[8] = Data_In & 0x0F;                   // Get lower nibble for month1 (one's place of month)
            time[9] = (Data_In & 0x08) >> 4;            // Get bit 5 for month2 (ten's place of month)
            break;
        case 6:
            time[10] = Data_In & 0x0F;                  // Get lower nibble for year1 (one's place of year)
            time[11] = (Data_In & 0xF0) >> 4;           // Get upper nibble for year2 (ten's place of year)
            break;
        default:
            break;
    }

}

/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

	WDTCTL = WDTPW | WDTHOLD;	    // Stop watchdog timer

	UCB0CTLW0 |= UCSWRST;           // Software reset for I2C
	//UCA1CTLW0 |= UCSWRST;         // Sofware reset for UART1

	// LED Defintiions (for testing)
	P1DIR |= BIT0;                  // Set P1.0 (LED1) as output
	P1OUT &= ~BIT0;                 // Start LED1 off

	P6DIR |= BIT6;                  // Set P6.6 (LED2) as output
	P6OUT &= ~BIT6;                 // Start LED2 off

	init_RTC();

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O

    UCB0CTLW0 &= ~UCSWRST;          // Take out of software reset for I2C
    //UCA1CTLW0 &= ~UCSWRST;        // Take out of software reset for UART1

    // Enable interrupts
    UCB0IE |= UCTXIE0 | UCRXIE0;    // Local interrupt enable for I2C TX0/RX0

    __enable_interrupt();           // Global IRQ enable

    while(1){

        getTime();

    }

	return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/

#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    if(alarm == 0) {                                                    // Get date/time information from RTC Registers

        switch(UCB0IV) {
            case 0x16:                                                  // Read from RTC Register
                Data_In = UCB0RXBUF;
                setTime();                                              // Store read value
                if(t1 == 6) {                                           // Reset register counter to 0 after all 6 date/time registers have been read from
                    t1 = 0;
                } else {                                                // Increase register counter to read from next date/time register
                    t1 = t1 + 1;
                    if(t1 == 3) {                                       // Skip reading RTC Register at 0x03 (ignore Day of Week)
                        t1 = t1 + 1;
                    }
                }
                break;
            case 0x18:                                                  // Set next RTC Register to read from
                UCB0TXBUF = t1;
                break;
            default:
                break;
        }

    } else if(alarm == 1) {                                             // Setup Alarm2 to occur every 15 minutes

        switch(UCB0IV) {
            case 0x16:
                break;
            case 0x18:
                UCB0TXBUF = 0x0B;                                       // Write to register at 0x0B in RTC to control Alarm2 minutes
                break;
            default:
                break;
        }

    }


}


/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: BLE UART1 Communication                */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_RX(void) {

    // Shift data being stored
    // Store newest data being received

    if(UCA1RXBUF == '1'){                                               // Toggle LED1 if '1' received (for testing)
        P1OUT ^= BIT0;
    }

    measurement1[u1] = UCA1RXBUF;                                       // Place received character into newest measurement

    if(u1 == 3) {                                                       // If four characters received, reset measurement index
        u1 = 0;
    } else {                                                            // Increment measurement index
        u1++;
    }

    UCA1IFG &= ~UCRXIFG;                                                // Clear flags for RX

}

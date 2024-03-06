/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Parent Device Program Integration Work                            */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/
unsigned int u1 = 0;                                    // Counter for UART1

/* Sensor Global Variables */
volatile int cycles;                                                            // Cycle counter for pulse measuring
volatile int measurement;                                                       // May not need
volatile int measurement_array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};              // Array for collecting 10 measurements
volatile int sensor_value;                                                      // Sensor measurement
volatile int wait = 0;                                                          // Indicator for sensor startup time
volatile int start = 0;                                                         // Indicator for pulse start/end
int prev_measurements[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                 // Array for storing previous 12 measurements


/* RTC Global Variables */
volatile unsigned int t1 = 0;
volatile unsigned int set = 0;
volatile unsigned int collect =0;
volatile unsigned int write = 1;
volatile char Data_In;
unsigned int count = 0;                                                                         // Counter for RTC Alarm Interrupt (count to 15 minutes)
char reset[] = {0x0F, 0x80};                                                                    // Transmit packet for resetting Alarm flag in RTC Control Register
char time[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};         // Time packet for receiving time/date from RTC registers
// Packet Format: {sec1, sec2, min1, min2, hour1, hour2, day1, day2, month1, month2, year}
// Packet Example: February 8th, 2024 at 2:43:13 -> {0x03, 0x01, 0x03, 0x04, 0x02, 0x0, 0x08, 0x00, 0x02, 0x00, 0x04, 0x02}

/*-------------------------------------------------------------------*/
/* Sensor I/O Initialization                                         */
/*-------------------------------------------------------------------*/
void init_sensor(void) {

    // PWM Pin Setup
    P2DIR &= ~BIT2;                 // Set P2.7 (PWM) as input
    P2REN |= BIT2;                  // Enable pull up/down resistors
    P2OUT |= BIT2;                  // Set as pull up resistor
    P2IES &= ~BIT2;                 // Low to High Sensitivity for PWM

    // Power Pin Setup
    //P2DIR |= BIT6;                  // Set P2.6 (Power) as output
    //P2OUT |= BIT6;                  // Start on (measuring)

}

/*-------------------------------------------------------------------*/
/* TimerB0 Initialization for Pulse Measuring                        */
/*-------------------------------------------------------------------*/
void init_sensorTimer(void) {

    TB0CTL |= TBCLR;                // Clear Timer B0
    TB0CTL |= TBSSEL__SMCLK;        // Select SMCLK
    TB0CTL |= MC__CONTINUOUS;       // Use continuous counting mode
    TB0CCTL0 |= CAP;
    TB0CCTL0 |= CM__BOTH;
    TB0CCTL0 |= CCIS__GND;

}

/*-------------------------------------------------------------------*/
/* TimerB1 Initialization for Sensor Startup                         */
/*-------------------------------------------------------------------*/
void init_startupTimer(void) {

    TB1CTL |= TBCLR;            // Clear Timer B0
    TB1CTL |= TBSSEL__ACLK;     // Select ACLK
    TB1CTL |= MC__UP;           // Using counting up mode
    TB1CCR0 = 6400;             // Set to count to 200ms

}

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
void initRTC(void) {

    UCB0CTLW0 |= UCMODE_3;                              // Put into I2C mode
    UCB0BRW |= 10;                                      // Prescalar = 10
    UCB0CTLW0 |= UCMST;                                 // Master mode
    UCB0CTLW0 &= ~UCTR;                                 // Read/RX mode
    UCB0I2CSA = 0x68;                                   // Slave Address = 0x68
    UCB0TBCNT = sizeof(reset);                          // Byte count (1 byte per register in RTC)
    UCB0CTLW1|= UCASTP_2;                               // Auto stop mode

    P1SEL1 &= ~BIT3;                                    // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;                                    // P1.2 = SDA
    P1SEL0 |= BIT2;

}

/*-------------------------------------------------------------------*/
/* Delay Function for Sensor Startup                                 */
/*-------------------------------------------------------------------*/
void delay(void) {

    TB1CCTL0 |= CCIE;               // Enable IRQ for Timer B1 CCR0
    TB1CCTL0 &= ~CCIFG;             // Clear flags for Timer B1 CCR0

    while(wait == 0) {}             // Wait for timer to indicate enough time has passed

    TB1CCTL0 &= ~CCIE;              // Disable TimerB1 IRQ
    TB1CCTL0 &= ~CCIFG;             // Clear flags for Timer B1 CCR0

}

/*-------------------------------------------------------------------*/
/* Collect Sensor Measurement                                        */
/*-------------------------------------------------------------------*/
void sensor_measurement(void) {

    int i, j, k, m;                     // Local variables for looping/counting
    m = 0;                              // Start measurement count at 0

    /* Collect 10 Measurements */

    for(i=0; i < 25; i++) {             // Collect multiple measurements

        P2IES &= ~BIT2;                 // LOW to HIGH sensitivity

        P2IE  |=  BIT2;                 // Enable IRQ for PWM I/O pin
        P2IFG &= ~BIT2;                 // Clear flags for PWM I/O pin


        // Getting stuck here
        while(start == 0){}             // Wait for start of pulse

        P2IES |= BIT2;                  // HIGH to LOW sensitivity
        P2IFG &= ~BIT2;                 // Clear flags for PWM

        TB0CCTL0 |= CCIE;               // Enable IRQ for Timer B0
        TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0

        while(start == 1){}             // Wait for PWM measurement

        P2IE  &= ~BIT2;                 // Disable IRQ for PWM
        P2IFG &= ~BIT2;                 // Clear flags for PWM

        TB0CCTL0 &= ~CCIE;              // Disable TimerB0 IRQ
        TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0


        if(cycles > 5 && m < 10) {                      // Collect 10 measurements, ignoring any zeros

            measurement_array[m] = cycles / 60;         // Convert cycles counted in pulse to centimeters and store in array (maybe 61)
            m = m+1;                                    // Increment measurement counter

        }

        cycles = 0;                     // Reset cycle count
        start = 0;                      // Reset start to indicate ready for next pulse
    }


    /* Sort Array Least to Greatest */

    int a, b, c;                        // Local variables for looping/counting

    for (a=1; a < 10; ++a){             // Iterate through all 10 values in array
        b = measurement_array[a];
        for (c = a - 1; (c >= 0) && (b < measurement_array[c]); c--){
            measurement_array[c + 1] = measurement_array[c];
        }
        measurement_array[c + 1] = b;
    }

    /* Return mode of array, or if no mode, return median */

    int duplicateCount = 0;
    int maxCount = 0;
    int prevCount = 0;
    int filter_value = 0;
    int median;

    a = 0;
    int x;
    int total_value = 0;

    while(a < 10){                                              // Go through array 10 times
    prevCount=duplicateCount;                                   // Store previous count of duplicates
    duplicateCount=0;                                           // Clear duplicate counter

        while(measurement_array[a]==measurement_array[a+1]){    // Count duplicate values
            duplicateCount++;
            a++;
        }

        if(duplicateCount>prevCount && duplicateCount>maxCount){    // Determine duplicate count is higher than previous or max count
            filter_value=measurement_array[a];
            maxCount=duplicateCount;
            median=0;
        }
        else if(duplicateCount==0){                                 // Do nothing if no duplicate found
            a++;
        }
        else if(duplicateCount==maxCount){                          // If max count reached, indicate no mode is found
            median=1;                                               // Assert median
        }

        if((filter_value == 0)||(median == 1)){                     // Find median if no mode was found
            filter_value = measurement_array[5];                    // Take 6th value in sorted measurement array as median
        }
    }

    sensor_value = filter_value;                                    // Set selected value (median or mode) as sensor measurement to store/send

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
            time[5] = (Data_In & 0x30) >> 4;            // Get bit 5 for hour2 (ten's place of hour)

            break;
        case 4:
            time[6] = Data_In & 0x0F;                   // Get lower nibble for day1 (one's place of day)
            time[7] = (Data_In & 0x30) >> 4;            // Get bits 5 & 6 for day2 (ten's place of day)
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
/* Get Time Function for Readint RTC Time Registers                  */
/*-------------------------------------------------------------------*/
void getTime(void) {

    int i;

    UCB0TBCNT = 1;                                      // Set byte count for reading one byte at a time
    write = 0;                                          // Indiciate reading from RTC

    for(i = 0; i < 6; i++) {                            // Read all 6 RTC time/date registers

        UCB0CTLW0 |= UCTR;                              // Set I2C to transmit to RTC (register address)
        UCB0CTLW0 |= UCTXSTT;                           // Send start message

        while((UCB0IFG & UCSTPIFG) == 0) {}             // Wait for data tranmission to complete
        UCB0IFG &= ~UCSTPIFG;                           // Clear flags

        UCB0CTLW0 &= ~UCTR;                             // Set I2C to receive from RTC (data at register address)
        UCB0CTLW0 |= UCTXSTT;                           // Send start message

        while((UCB0IFG & UCSTPIFG) == 0) {}             // Wait for data transmission to complete
        UCB0IFG &= ~UCSTPIFG;                           // Clear flags

    }

}

/*-------------------------------------------------------------------*/
/* Clear RTC Alarm 2 Flag                                            */
/*-------------------------------------------------------------------*/
void clearAlarm(void) {


    UCB0TBCNT = sizeof(reset);                          // Set byte count to size of reset packet
    write = 1;                                          // Indicate writing to RTC

    UCB0CTLW0 |= UCTR;                                  // Set I2C to transmit to RTC
    UCB0CTLW0 |= UCTXSTT;                               // Send start message

    while((UCB0IFG & UCSTPIFG) == 0) {}                 // Wait for data transmission to complete
    UCB0IFG &= ~UCSTPIFG;                               // Clear flags

    P3IE |= BIT0;                                       // Enable IRQ for SQW once I2C communication is complete
    set = 0;                                            // Indicate alarm flag no longer set

}


/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    UCB0CTLW0 |= UCSWRST;                               // Software reset for I2C

    /* I/O Defintiions */
    P1DIR |= BIT0;                                      // Set P1.0 (LED1) as output
    P1OUT &= ~BIT0;                                     // Start LED1 off

    P3DIR &= ~BIT0;                                     // Set P3.0 (SQW) as input
    P3REN |= BIT0;                                      // Enable pull up/down resistors
    P3OUT |= BIT0;                                      // Set as pull up resistor
    P3IES |= BIT0;                                      // High to Low Sensitivity for SQW

    initRTC();                                          // Initialize I2C settings for RTC communication
    init_sensor();                                      // Initialize I/O settings for sensor
    init_sensorTimer();                                 // Initialize timer settings for PWM measurement
    init_startupTimer();                                // Intialize timer setting for sensor startup

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCB0CTLW0 &= ~UCSWRST;                              // Take out of software reset for I2C

    /* Enable interrupts */
    UCB0IE |= UCTXIE0 | UCRXIE0;;                       // Local interrupt enable for I2C TX0

    P3IE |= BIT0;                                       // Enable IRQ for SQW
    P3IFG &= ~BIT0;                                     // Clear flags for SQW

    __enable_interrupt();                               // Global IRQ enable

    getTime();
    clearAlarm();                                       // Start with Alarm 2 flag cleared in RTC Control Register
    delay();                                            // Wait for sensor start up

    while(1){                                           // Wait for interrupt to occur

        if(set == 1) {                                  // Get time from RTC and clear Alarm 2 flag if it has been set
            getTime();
            clearAlarm();
        }

        if(collect == 1) {
            delay();
            sensor_measurement();                                           // Get parent sensor measurement
            // Call for BLE to alert child here
            collect = 0;
        }

    }

    return 0;
}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM I/O                                */
/*-------------------------------------------------------------------*/
#pragma vector = PORT2_VECTOR
__interrupt void ISR_PWM(void){

    if(start == 0) {                    // Pulse measurement start

        TB0R = 0;                       // Clear TB0 count register
        start = 1;
        TB0CCTL0 |= CCIS__VCC;

    } else if(start == 1) {             // Pulse measurement end

        cycles = TB0R;                  // Collect TB0R count value
        start = 0;
        TB0CCTL0 |= CCIS__GND;
    }

    P2IFG &= ~BIT2;                     // Clear interrupt flag

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM Timer B0                           */
/*-------------------------------------------------------------------*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void){

    TB0CCTL0 &= ~CCIFG;                 // Clear Timer B0 flag

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: Startup Timer B1                       */
/*-------------------------------------------------------------------*/
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void){

    wait = 1;                           // Indicate enough time has passed
    TB1CCTL0 &= ~CCIFG;                 // Clear Timer B1 flag

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC I2C Communication                  */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){

    if(write == 1) {                                                    // I2C Communication for Writing to RTC
        UCB0TXBUF = reset[t1];                                          // Place next byte into I2C TX buffer
        t1 = t1 + 1;                                                    // Increment to next byte in packet
        if(t1 == sizeof(reset)) {                                       // Determine if entire packet has been sent
            t1 = 0;                                                     // Reset byte counter
            UCB0IFG &= ~UCTXIFG0;                                       // Clear flag to allow I2C interrupt
        }
    } else {                                                            // I2C Communication for
        switch(UCB0IV) {                                                // Determine if transmitting address or receiving data
            case 0x16:
                Data_In = UCB0RXBUF;                                    // Place received data from buffer in temporary variable
                setTime();                                              // Use data to update time packet
                if(t1 == 6) {                                           // Reset register counter to 0 after all 6 date/time registers have been read from
                    t1 = 0;
                } else {                                                // Increase register counter to read from next date/time register
                    t1 = t1 + 1;
                    if(t1 == 3) {                                       // Skip reading RTC Register at 0x03 (ignore Day of Week)
                        t1 = t1 + 1;
                    }
                }
                break;
            case 0x18:
                UCB0TXBUF = t1;                                         // Transmit address to read from
                break;
        }
    }

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: RTC Alarm                              */
/*-------------------------------------------------------------------*/
#pragma vector = PORT3_VECTOR
__interrupt void PORT3_ISR(void){

    set = 1;                                                            // Indiciate Alarm Flag has been set in RTC Control Register
    //if(count < 15) {                                                    // Increase counter if 15 minutes haven't passed
    if(count < 1) {
        count = count + 1;
    } else {                                                            // Clear counter and call for measurement if 15 minutes have passed
        P1OUT ^= BIT0;
        count = 0;
        collect = 1;
    }

    P3IE &= ~BIT0;                                                      // Temporarily disable IRQ for SQW
    P3IFG &= ~BIT0;                                                     // Clear flags for SW1


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

    //measurement1[u1] = UCA1RXBUF;                                       // Place received character into newest measurement

    if(u1 == 3) {                                                       // If four characters received, reset measurement index
        u1 = 0;
    } else {                                                            // Increment measurement index
        u1++;
    }

    UCA1IFG &= ~UCRXIFG;                                                // Clear flags for RX

}

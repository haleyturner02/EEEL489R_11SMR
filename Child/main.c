/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Child Device Program                                              */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/*-------------------------------------------------------------------*/
/* Variable Definitions for Getting Measurement                      */
/*-------------------------------------------------------------------*/

/* Sensor Global Variables */
volatile int cycles;                                                                            // Cycle counter for pulse measuring
volatile int measurement_array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                              // Array for collecting 10 measurements
volatile int sensor_value;                                                                      // Sensor measurement
volatile int wait = 0;                                                                          // Indicator for sensor startup time
volatile int start = 0;                                                                         // Indicator for pulse start/end

/* BLE UART Global Variables */
volatile unsigned int received = 0;                                                             // Character received indicator
volatile unsigned char receive_data = 0x00;                                                     // Character received
volatile unsigned char measurement[] = {0x28, 0x30, 0x29, 0x5B, 0x30, 0x5D, 0x7B, 0x30, 0x7D};  // Packet for measurement transmission

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
    P2DIR |= BIT6;                  // Set P2.6 (Power) as output
    P2OUT |= BIT6;                  // Start on (measuring)

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
/* UART0 Initialization for BLE Communication                        */
/*-------------------------------------------------------------------*/
void init_BLE() {
    // Settings to acheive baud rate of 115200

    UCA0CTLW0 |= UCSSEL__SMCLK;         // Use SMCLK
    UCA0BRW = 8;                        // Prescaler = 8
    UCA0MCTLW = 0xD600;                 // Set modulation and low frequency

    P1SEL1 &= ~BIT6;                    // Set P1.6 to UART A0 RX
    P1SEL0 |= BIT6;

    P1SEL1 &= ~BIT7;                    // Set P1.7 UART A0 TX
    P1SEL0 |= BIT7;

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
/* Format Packet with New Measurement                                */
/*-------------------------------------------------------------------*/
void format_packet(){

    int hundreds, tens, ones, temp;

    temp = sensor_value;

    if(temp > 99){
        hundreds = sensor_value - (sensor_value % 100);
    } else {
        hundreds = 0;
    }

    temp = temp - hundreds;

    if(temp > 9){
        tens = (sensor_value % 100) - (sensor_value % 10);
    } else {
        tens = 0;
    }

    temp = temp - tens;
    ones = temp;

    switch(hundreds){
        case 0:
            measurement[1] = 0x30;
            break;
        case 100:
            measurement[1] = 0x31;
            break;
        case 200:
            measurement[1] = 0x32;
            break;
        case 300:
            measurement[1] = 0x33;
            break;
        case 400:
            measurement[1] = 0x34;
            break;
        case 500:
            measurement[1] = 0x35;
            break;
        case 600:
            measurement[1] = 0x36;
            break;
        case 700:
            measurement[1] = 0x37;
            break;
        default:
            measurement[1] = 0x00;
            break;

    }

    switch(tens){
        case 0:
            measurement[4] = 0x30;
            break;
        case 10:
            measurement[4] = 0x31;
            break;
        case 20:
            measurement[4] = 0x32;
            break;
        case 30:
            measurement[4] = 0x33;
            break;
        case 40:
            measurement[4] = 0x34;
            break;
        case 50:
            measurement[4] = 0x35;
            break;
        case 60:
            measurement[4] = 0x36;
            break;
        case 70:
            measurement[4] = 0x37;
            break;
        case 80:
            measurement[4] = 0x38;
            break;
        case 90:
            measurement[4] = 0x39;
            break;
        default:
            measurement[4] = 0x00;
            break;
    }

    measurement[7] = ((unsigned char) ones) + 48;

}

/*-------------------------------------------------------------------*/
/* Send Measurement to BLE via UART0                                 */
/*-------------------------------------------------------------------*/
void send_measurement(){

    int i, j, n;

    for(j = 0; j < 3; j++){

        for(n = 0; n < sizeof(measurement); n++){
            UCA0TXBUF = measurement[n];
            for(i = 0; i < 1000; i++){}
        }
    }

}

/*-------------------------------------------------------------------*/
/* Main Function for Inializing and Waiting for Interrupts           */
/*-------------------------------------------------------------------*/
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;                           // Stop watchdog timer
    UCA0CTLW0 |= UCSWRST;                               // Put into software reset for UART0

    init_sensor();                                      // Initialize I/O settings for sensor
    init_sensorTimer();                                 // Initialize timer settings for PWM measurement
    init_startupTimer();                                // Intialize timer setting for sensor startup
    init_BLE();

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCA0CTLW0 &= ~UCSWRST;                              // Take out of software reset for UART0

    UCA0IE |= UCRXIE;                                   // Enable UART0 RX interrupt

    __enable_interrupt();                               // Global IRQ enable

    while(1){

        sensor_value = 304;
        format_packet();
        send_measurement();

        UCA0IFG &= ~UCRXIFG;
        while(received == 0){}
        received = 0;

        if(receive_data == 0x23){
            UCA0IE &= ~UCRXIE;
            //delay();
            //sensor_measurement();
            format_packet();
            send_measurement();
            UCA0IE |= UCRXIE;
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
/* Interrupt Service Routine: BLE UART1 Communication                */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCI_A0_UART_ISR(void) {

    receive_data = UCA0RXBUF;
    received = 1;
    UCA0IFG &= ~UCRXIFG;

}

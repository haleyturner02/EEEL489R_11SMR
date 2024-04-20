/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Parent Device Program- Snow Depth Reporting                       */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/

/* Sensor Global Variables */
volatile int cycles;                                                                            // Cycle counter for pulse measuring
volatile int measurement_array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};                              // Array for collecting 10 measurements
int parent_calibration = 0;                                                                     // Initial parent sensor measurement
int parent_value = 0xCF;                                                               // Latest parent sensor measurement
int child1_calibration = 0;                                                                     // Initial child 1 sensor measurement
int child1_value = 0xCF;                                                               // Child 1 sensor measurement
int child2_calibration = 0;                                                                     // Initial child 2 sensor measurement
int child2_value = 0xCF;                                                               // Child 2 sensor measurement
volatile int wait = 0;                                                                          // Indicator for sensor startup time
volatile int start = 0;                                                                         // Indicator for pulse start/end

/* RTC Global Variables */
volatile unsigned int t1 = 0;                                                                   // Value for reading time/date and storing in packet
volatile unsigned int t2 = 0;                                                                   // Value for reading temperature and storing in packet
volatile unsigned int set = 0;                                                                  // Indicator for alarm occurrence
volatile unsigned int collect = 0;                                                              // Indicator for collecting measurements
volatile unsigned int send = 0;                                                                 // Indicator for sending measurements
volatile unsigned int write = 1;                                                                // Indicator for reading or writing to RTC (1 -> write, 0 -> read)
volatile unsigned int temp = 0;                                                                 // Indicator for reading time or temperature (1 -> temperature, 0 -> time)
volatile unsigned int operation = 1;                                                            // Indicator for operation (1 -> normal operation, 0 -> too cold to operate)
volatile char Data_In;
unsigned int count = 0;                                                                         // Counter for RTC Alarm Interrupt (count to 15 minutes)
char reset[] = {0x0F, 0x80};                                                                    // Transmit packet for resetting Alarm flag in RTC Control Register
char time[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};         // Time packet for receiving time/date from RTC registers
// Packet Format: {sec1, sec2, min1, min2, hour1, hour2, day1, day2, month1, month2, year}
// Packet Example: February 8th, 2024 at 2:43:13 -> {0x03, 0x01, 0x03, 0x04, 0x02, 0x0, 0x08, 0x00, 0x02, 0x00, 0x04, 0x02}
int temperature[] = {0, 0, 0, 0, 0};                                                            // Temperature packet for receiving temperature from RTC registers
// Packet Format: {frac1, frac2, temp1, temp2, sign}
// Packet Example: +21.75 degrees Celsius -> {5, 7, 1, 2, 0}

/* BLE UART1 Global Variables */
volatile int retry = 0;                                                                         // Retry BLE connection indicator
volatile unsigned int receiving = 0;                                                            // Receiving state indicator (for measurement)
volatile unsigned int received = 0;                                                             // Character received indicator
volatile unsigned char receive_data [] = {0x3C, 0x30, 0x30, 0x30, 0x3E};                        // Packet for received measurement
// Packet Format: {'<', hex1, hex2, hex3, '>'}
// Packet Example: 0x23A -> {0x3C, 0x32, 0x33, 0x3A, 0x3D}

/* Transceiver UART0 Global Variables */
unsigned int baseSend = 0;
unsigned char clusterID = 0x01;                                                                 // Cluster ID
volatile unsigned char data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// Packet Format: {clusterID, minutes, hours, depth (parent), depth (child #1), depth (child #2)}
// Packet Example: Cluster #1 at 13:46 with measurement of 180cm (parent), 179cm (child #1), 181cm (child #2) {0x01, 0x2E, 0x0D, 0xB4, 0xB3, 0xB5}


/*-------------------------------------------------------------------*/
/* Calibration Button Initialization                                 */
/*-------------------------------------------------------------------*/
void init_button(void){

    P1DIR &= ~BIT1;                                 // Set P1.1 (button) as input
    P1IES &= ~BIT1;                                 // Low to High Sensitivity for button

}

/*-------------------------------------------------------------------*/
/* Sensor I/O Initialization                                         */
/*-------------------------------------------------------------------*/
void init_sensor(void) {

    // PWM Pin Setup
    P2DIR &= ~BIT7;                 // Set P2.7 (PWM) as input
    P2REN |= BIT7;                  // Enable pull up/down resistors
    P2OUT |= BIT7;                  // Set as pull up resistor
    P2IES &= ~BIT7;                 // Low to High Sensitivity for PWM

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
/* I2C Initialization for RTC Communication                          */
/*-------------------------------------------------------------------*/
void init_RTC(void) {

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

    P3DIR &= ~BIT0;                                     // Set P3.0 as input (Alarm)
    P3REN |= BIT0;                                      // Enable pull up/down resistors
    P3OUT |= BIT0;                                      // Set as pull up resistor
    P3IES |= BIT0;                                      // High to Low Sensitivity for SQW

}

/*-------------------------------------------------------------------*/
/* UART1 Initialization for BLE Communication                        */
/*-------------------------------------------------------------------*/
void init_BLE() {
    // Settings to acheive baud rate of 115200

    UCA1CTLW0 |= UCSSEL__SMCLK;                         // Use SMCLK
    UCA1BRW = 8;                                        // Prescaler = 8
    UCA1MCTLW = 0xD600;                                 // Set modulation and low frequency

    P4SEL1 &= ~BIT2;                                    // Set P4.2 to UART A1 RX
    P4SEL0 |= BIT2;

    P4SEL1 &= ~BIT3;                                    // Set P4.3 UART A1 TX
    P4SEL0 |= BIT3;

    P6DIR &= ~BIT2;                                     // Set P6.2 Reset

}

/*-------------------------------------------------------------------*/
/* UART0 Initialization for Transceiver Communication                */
/*-------------------------------------------------------------------*/
void init_Transceiver() {
    // Settings to acheive baud rate of 115200

    UCA0CTLW0 |= UCSSEL__SMCLK;                         // Use SMCLK
    UCA0BRW = 8;                                        // Prescaler = 8
    UCA0MCTLW = 0xD600;                                 // Set modulation and low frequency

    P1SEL1 &= ~BIT6;                                    // Set 1.6 to UART A0 RX
    P1SEL0 |= BIT6;

    P1SEL1 &= ~BIT7;                                    // Set P1.7 UART A0 TX
    P1SEL0 |= BIT7;

}

/*-------------------------------------------------------------------*/
/* Delay Function for Sensor Startup                                 */
/*-------------------------------------------------------------------*/
void delay(void) {

    TB1CCTL0 |= CCIE;                                   // Enable IRQ for Timer B1 CCR0
    TB1CCTL0 &= ~CCIFG;                                 // Clear flags for Timer B1 CCR0

    while(wait == 0) {}                                 // Wait for timer to indicate enough time has passed
    wait = 0;

    TB1CCTL0 &= ~CCIE;                                  // Disable TimerB1 IRQ
    TB1CCTL0 &= ~CCIFG;                                 // Clear flags for Timer B1 CCR0

}

/*-------------------------------------------------------------------*/
/* Collect Sensor Measurement                                        */
/*-------------------------------------------------------------------*/
void sensor_measurement(void) {

    int i, j, k, m;                                                 // Local variables for looping/counting
    m = 0;                                                          // Start measurement count at 0

    /* Collect 10 Measurements */

    for(i=0; i < 25; i++) {                                         // Collect multiple measurements

        P2IES &= ~BIT7;                                             // LOW to HIGH sensitivity

        P2IE  |=  BIT7;                                             // Enable IRQ for PWM I/O pin
        P2IFG &= ~BIT7;                                             // Clear flags for PWM I/O pin

        while(start == 0){}                                         // Wait for start of pulse

        P2IES |= BIT7;                                              // HIGH to LOW sensitivity
        P2IFG &= ~BIT7;                                             // Clear flags for PWM

        TB0CCTL0 |= CCIE;                                           // Enable IRQ for Timer B0
        TB0CCTL0 &= ~CCIFG;                                         // Clear flags for Timer B0

        while(start == 1){}                                         // Wait for PWM measurement

        P2IE  &= ~BIT7;                                             // Disable IRQ for PWM
        P2IFG &= ~BIT7;                                             // Clear flags for PWM

        TB0CCTL0 &= ~CCIE;                                          // Disable TimerB0 IRQ
        TB0CCTL0 &= ~CCIFG;                                         // Clear flags for Timer B0


        if(cycles > 5 && m < 10) {                                  // Collect 10 measurements, ignoring any zeros

            measurement_array[m] = cycles / 60;                     // Convert cycles counted in pulse to centimeters and store in array (maybe 61)
            m = m+1;                                                // Increment measurement counter

        }

        cycles = 0;                                                 // Reset cycle count
        start = 0;                                                  // Reset start to indicate ready for next pulse
    }


    /* Sort Array Least to Greatest */

    int a, b, c;                                                    // Local variables for looping/counting

    for (a=1; a < 10; ++a){                                         // Iterate through all 10 values in array
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

    while(a < 10){                                                  // Go through array 10 times
    prevCount=duplicateCount;                                       // Store previous count of duplicates
    duplicateCount=0;                                               // Clear duplicate counter

        while(measurement_array[a]==measurement_array[a+1]){        // Count duplicate values
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

    parent_value = filter_value;

}

/*-------------------------------------------------------------------*/
/* Temperature Compensation for Sensor Measurement                   */
/*-------------------------------------------------------------------*/
void temperatureCompensation(int device){

    float val, t, result;

    if(device == 0) {                               // Determine device to compensate value for
        val = parent_value;
    } else if (device == 1) {
        val = child1_value;
    } else if (device == 2) {
        val = child2_value;
    }

    t = (temperature[3] * 10) + temperature[2] + (temperature[1] * 0.1) + (temperature[0] * 0.01);      // Get temperature for compensation

    if(temperature[4] == 1) {                       // Determine if temperature is negative
        t = -t;
    }

    result = ((331.2 + (t*0.61))*val)/345;          // Compensate value

    val = ((int) result) + 0.5;                     // Get whole portion of compensation + 0.5

    if(val <= result){                              // Round compensated value to nearest whole number
        result = result + 1;
    }

    if(device == 0) {                               // Set device value to compensated result
        parent_value = result;
    } else if (device == 1) {
        child1_value = result;
    } else if (device == 2) {
        child2_value = result;
    }

}

/*-------------------------------------------------------------------*/
/* BLE Module Reset                                                  */
/*-------------------------------------------------------------------*/
void bleReset(){

    int i;

    P6DIR |= BIT2;                                  // Set P6.2 to output for BLE reset
    P6OUT |= BIT2;                                  // Set P6.2 HIGH for active high reset
    for(i = 0; i < 1000; i++){}                     // Small delay to wait for reset
    P6DIR &= ~ BIT2;
    for(i = 0; i < 10; i++) {                       // Delay to wait for BLE to restart
        delay();
    }

}

/*-------------------------------------------------------------------*/
/* Request Value from Child Device                                   */
/*-------------------------------------------------------------------*/
void requestValue(int device){

    int i;

    receiving = 0;                              // Reset receiving state indicator for next measurement reception

    if(device == 1) {
        UCA1TXBUF = 0x23;                       // Send request to BLE for Child 1 value
    }
    for(i = 0; i < 1000; i++){}
    UCA1IE |= UCRXIE;                           // Enable UART1 RX interrupt for measurement reception

}

/*-------------------------------------------------------------------*/
/* Send Data Packet to transceiver                                   */
/*-------------------------------------------------------------------*/
void sendToBase(){

    int i, j;

    getTime();

    data[0] = clusterID;                                    // Include cluster ID in data packet
    data[1] = time[3]*10 + time[2];                         // Include min in data packet
    data[2] = time[5]*10 + time[4];                         // Include hour in data packet

    if(parent_value < 0) {
        parent_value = 0;
    } else if(parent_value < 207) {
        data[3] = parent_value;                             // Include parent snow depth in data packet
    } else {
        data[3] = 0xCF;                                     // No value/value out of range
    }

    if(child1_value < 0) {
        child1_value = 0;
    } else if(child1_value < 207) {
        data[4] = child1_value;                             // Include child 1 snow depth in data packet
    } else {
        data[4] = 0xCF;                                     // No value/value out of range
    }

    if(child2_value < 0) {
        child2_value = 0;
    } else if(child2_value < 207) {
        data[5] = child2_value;                             // Include child 2 snow depth in data packet
    } else {
        data[5] = 0xCF;                                     // No value/value out of range
    }

    data[5] = 0xCF;                                         // Hard set Child 2 value (no value)

    for(j = 0; j < sizeof(data); j++){
        UCA0TXBUF = data[j] + 48;                           // Send each value in data packet to transceiver to transmit to base
        for(i = 0; i < 1000; i++){}

    }

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
/* Get Time Function for Reading RTC Time Registers                  */
/*-------------------------------------------------------------------*/
void getTime(void) {

    int i;

    UCB0TBCNT = 1;                                      // Set byte count for reading one byte at a time
    write = 0;                                          // Indiciate reading from RTC
    temp = 0;                                           // Indicate reading time from RTC

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
/* Check Temperature Function for determining operation              */
/*-------------------------------------------------------------------*/
void checkTemp(void) {


    if(temperature[4] == 1){                            // Check if temperature is negative
        if(temperature[3] >= 2) {                       // Check if temperature is <= -20 degrees
            if(temperature[2] >= 3) {                   // Check if temperature is <= -23 degrees
                operation = 0;
            } else if (temperature[2] == 2 && temperature[1] >= 5) {    // Check if temperature is <= -22.5 degrees
                operation = 0;
            } else {
                operation = 1;
            }
        } else {
            operation = 1;
        }
    } else {
        operation = 1;                                  // If temperature is positive, continue normal operation
    }

}

/*-------------------------------------------------------------------*/
/* Set Temperature Function for storing temp from RTC in packet      */
/*-------------------------------------------------------------------*/
void setTemp(void) {

    int integer, fractional;                            // Get integer and fractional portions of temperature

    switch(t2) {
        case 0:
            integer = Data_In & 0x7F;                   // Get lower 7 bits for integer
            if(integer > 9){                            // If integer is greater than 9, break into tens and ones digits
                temperature[2] = integer % 10;
                temperature[3] = (integer - (integer % 10))/10;
            } else {                                    // If integer is less than 1, store value in ones digit and set tens digit to zero
                temperature[2] = integer;
                temperature[3] = 0;
            }
            temperature[4] = (Data_In & 0x80) >> 7;     // Get MSB for sign bit
            break;
        case 1:
            fractional = (Data_In & 0xC0) >> 6;         // Get upper 2 bits for fractional
            switch(fractional){                         // Determine fraction portion
                case 0:                                 // 00 -> XX.00
                    temperature[0] = 0;
                    temperature[1] = 0;
                    break;
                case 1:                                 // 01 -> XX.25
                    temperature[0] = 5;
                    temperature[1] = 5;
                    break;
                case 2:                                 // 10 -> XX.50
                    temperature[0] = 0;
                    temperature[1] = 5;
                    break;
                case 3:                                 // 11 -> XX.75
                    temperature[0] = 5;
                    temperature[1] = 7;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

}

/*-------------------------------------------------------------------*/
/* Get Temperature Function for Reading RTC Temperature Registers    */
/*-------------------------------------------------------------------*/
void getTemp(void) {

    int i;

    UCB0TBCNT = 1;                                      // Set byte count for reading one byte at a time
    write = 0;                                          // Indiciate reading from RTC
    temp = 1;                                           // Indicate reading temperature from RTC

    for(i = 0; i < 2; i++) {                            // Read both RTC temperature registers

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
    UCA0CTLW0 |= UCSWRST;                               // Put into software reset for UART0
    UCA1CTLW0 |= UCSWRST;                               // Put into software reset for UART1
    UCB0CTLW0 |= UCSWRST;                               // Put into software reset for I2C

    /* Initialize Ports/Pins */
    init_button();
    init_sensor();                                      // Initialize I/O settings for sensor
    init_sensorTimer();                                 // Initialize timer settings for PWM measurement
    init_startupTimer();                                // Intialize timer setting for sensor startup
    init_RTC();                                         // Initialize I2C settings for RTC communication
    init_BLE();                                         // Initialize UART1 settings for BLE communication
    init_Transceiver();                                 // Initialize UART0 settings for Transceiver communication

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCA0CTLW0 &= ~UCSWRST;                              // Take out of software reset for UART0
    UCA1CTLW0 &= ~UCSWRST;                              // Take out of software reset for UART1
    UCB0CTLW0 &= ~UCSWRST;                              // Take out of software reset for I2C

    /* Enable interrupts */
    UCB0IE |= UCTXIE0 | UCRXIE0;                        // Interrupt enable for I2C

    P1IE |= BIT1;                                       // Enable IRQ for button calibration
    P1IFG &= ~BIT1;                                     // Clear flags for button

    P3IE |= BIT0;                                       // Enable IRQ for RTC Alarm
    P3IFG &= ~BIT0;                                     // Clear flags for RTC Alarm

    __enable_interrupt();                               // Global IRQ enable

    getTime();                                          // Collect initial RTC date/time
    getTemp();                                          // Collect initial RTC temperature
    clearAlarm();                                       // Start with Alarm 2 flag cleared in RTC Control Register

    int i;

    receiving = 6;

    while(1){

        if(set == 1) {                                          // Get date/time and temperature from RTC and clear Alarm 2 flag if it has been set

            getTime();
            clearAlarm();

            if(retry < 2 && receiving != 6){
                bleReset();                                     // Reset the BLE module
                requestValue(1);                                // Request Child 1 value
                retry = retry + 1;
            }

            if(collect == 1) {                                  // Collect measurements if RTC indicates 15 minutes have passed

                bleReset();                                     // Reset the BLE module
                getTemp();                                      // Collect temperature reading from RTC
                checkTemp();                                    // Check if temperature is within operation range
                if(operation == 1){

                    sensor_measurement();                       // Collect Parent value
                    //temperatureCompensation(0);               // Apply temperature compensation for Parent value
                    if(parent_calibration == 0) {               // Store first Parent value as device calibration distance
                        parent_calibration = parent_value;
                    }
                    parent_value = parent_calibration - parent_value;   // Compute Parent snow depth

                    requestValue(1);                            // Request Child 1 value
                }
                collect = 0;                                    // Clear measurement collection indicator
            }

            if(send == 1) {
                if(receiving != 6) {                            // Value from Child 1 not received
                    data[4] = 0xCF;
                }

                if(baseSend == 2){
                    sendToBase();                                   // Send data to base station
                    baseSend = 0;
                } else {
                    baseSend = baseSend + 1;
                }

                retry = 0;
                send = 0;                                       // Clear send data indicator
            }
        }

        if(receiving == 5) {                                    // Send values to base once Child Measurement has been received

            // Collect and Store Child 1 Measurement
            UCA1IE &= ~UCRXIE;                                  // Disable UART1 RX interrupt

            child1_value = (receive_data[1]-48)*100 + (receive_data[2]-48)*10 + (receive_data[3]-48);   // Store received value from Child

            if(child1_calibration == 0) {                       // Store first Child 1 value as device calibration distance
                child1_calibration = child1_value;
            }

            child1_value = child1_calibration - child1_value;   // Compute Child 1 snow depth
            //temperatureCompensation(1);                       // Apply temperature compensation for Child 1 value
            receiving = 6;                                      // Indicate Child 1 measurement has been received
            retry = 0;                                          // No need to rety BLE connection


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

    P2IFG &= ~BIT7;                     // Clear interrupt flag
    TB0CCTL0 |= CCIS__GND;

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
                if(temp == 0){                                          // Functionality for reading time
                    setTime();                                          // Use data to update time packet
                    if(t1 == 6) {                                       // Reset register counter to 0 after all 6 date/time registers have been read from
                        t1 = 0;
                    } else {                                            // Increase register counter to read from next date/time register
                        t1 = t1 + 1;
                        if(t1 == 3) {                                   // Skip reading RTC Register at 0x03 (ignore Day of Week)
                            t1 = t1 + 1;
                        }
                    }
                } else if (temp == 1) {                                 // Functionality for reading temperature
                    setTemp();                                          // Use data to update temperature packet
                    if(t2 == 1) {                                       // Reset register counter to 0 after both temperature registers have been read from
                        t2 = 0;
                    } else {                                            // Increase register counter to read from next temperature register
                        t2 = t2 + 1;
                    }
                }
                break;
            case 0x18:
                if(temp == 0){                                          // Determine if reading time or temperature
                    UCB0TXBUF = t1;                                     // Transmit time register address to read from
                } else if (temp == 1){
                    UCB0TXBUF = t2 + 17;                                // Transmit temp register address to read from
                }
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

    if(count < 4) {                                                    // Increase counter if 15 minutes haven't passed
        count = count + 1;
        if(count == 1) {
            collect = 1;                                                // Set indicator to collect values after 12 minutes
        }
    } else {                                                            // Clear counter and send data to base station after 15 minutes
        count = 0;
        send = 1;
    }

    P3IE &= ~BIT0;                                                      // Temporarily disable IRQ for SQW
    P3IFG &= ~BIT0;                                                     // Clear flags for SW1


}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: BLE UART1 Communication                */
/*-------------------------------------------------------------------*/
#pragma vector = EUSCI_A1_VECTOR
__interrupt void EUSCI_A1_RX_ISR(void) {

    if(receiving == 0 && UCA1RXBUF == 0x3C) {                       // State 0: Receiving = 0 -> wait for '<' to be received to indicate start of data
        receiving = 1;
    } else if(receiving == 1) {                                     // State 1: Receiving = 1 -> receive hundreds place value of measurement
        receive_data[1] = UCA1RXBUF;
        receiving = 2;
    } else if (receiving == 2) {                                    // State 2: Receiving = 2 -> receive tens place value of measurement
        receive_data[2] = UCA1RXBUF;
        receiving = 3;
    } else if (receiving == 3) {                                    // State 3: Receiving = 3 -> receive ones place value of measurement
        receive_data[3] = UCA1RXBUF;
        receiving = 4;
    } else if (receiving == 4 && UCA1RXBUF == 0x3E) {               // State 4: Receiving = 4 -> wait for '>' to be received to indicate end of data
        receiving = 5;

    }

    received = 1;                                                   // Set character received indicator
    UCA1IFG &= ~UCRXIFG;                                            // Clear UART1 RX flag

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: Button Calibration                     */
/*-------------------------------------------------------------------*/
#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void){

    parent_calibration = 0;                                         // Reset all device calibration values to 0 so that next measurement taken from each is the new calibration value
    child1_calibration = 0;
    child2_calibration = 0;

    P1IFG &= ~BIT1;                                                 // Clear flags for button

}

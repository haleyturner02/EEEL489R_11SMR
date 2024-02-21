/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Sensor Setup and Testing                                          */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/* Sensor Global Variables */

volatile int cycles;                                                            // Cycle counter for pulse measuring
volatile int measurement;                                                       // May not need
volatile int measurement_array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};              // Array for collecting 10 measurements
volatile int sensor_value;                                                      // Sensor measurement
volatile int wait = 0;                                                          // Indicator for sensor startup time
volatile int start = 0;                                                         // Indicator for pulse start/end

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

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    init_sensor();                  // Initialize I/O settings for sensor
    init_sensorTimer();             // Initialize timer settings for PWM measurement
    init_startupTimer();            // Intialize timer setting for sensor startup

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O

    __enable_interrupt();           // Global IRQ enable

    delay();                        // Wait for sensor to power on

    while(1){
        sensor_measurement();       // Get measurement from sensor (move this to interrupt)
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

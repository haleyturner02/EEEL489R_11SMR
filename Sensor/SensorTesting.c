/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Sensor Setup and Testing                                          */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

volatile int cycles;
volatile int measurement;
volatile int measurement_array[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int sensor_value;
volatile int wait = 0;
volatile int start = 0;

/*-------------------------------------------------------------------*/
/* Sensor I/O Initialization                                         */
/*-------------------------------------------------------------------*/
void init_sensor(void) {

    // PWM Pin Setup
    P2DIR &= ~BIT2;                 // Set P2.7 (PWM) as input
    P2REN |= BIT2;                  // Enable pull up/down resistors
    P2OUT |= BIT2;                  // Set as pull up resistor
    P2IES &= ~BIT2;                 // Low to High Sensitivity for PWM

/*
    // Power Pin Setup
    P2DIR |= BIT6;                  // Set P2.6 (Power) as output
    P2OUT |= BIT6;                  // Start on (measuring)
*/

 /*   // SW2 for testing
    P2DIR &= ~BIT3;
    P2REN |= BIT3;
    P2OUT |= BIT3;

  */

    //P2IES |= BIT3;                 // Start Low to High

    //P2IES |= BIT2;


}

/*-------------------------------------------------------------------*/
/* TimerB0 Initialization                                            */
/*-------------------------------------------------------------------*/
void init_sensorTimer(void) {

    TB0CTL |= TBCLR;              // Clear Timer B0
    //TB0CTL |= MC__UP;             // Using counting up mode

    // Option #1
    //TB0CTL |= TBSSEL__ACLK;
    //TB0CCR0 = 1421;

    // Option #2
    //TB0CTL |= TBSSEL__SMCLK;
    //TB0CCR0 = 44000;

    //Option #3
    //TB0CTL |= TBSSEL__ACLK;
    //TB0CTL |= ID__2;
    //TB0CCR0 = 4400;

    // Option #4
    //TB0CTL |= ID__8;
    //TB0CCR0 = 577;


    // Option #5 continuous mode
    TB0CTL |= TBSSEL__SMCLK;
    TB0CTL |= MC__CONTINUOUS;
    TB0CCTL0 |= CAP;
    TB0CCTL0 |= CM__BOTH;
    TB0CCTL0 |= CCIS__GND;

}

void init_startupTimer(void) {

    TB1CTL |= TBCLR;            // Clear Timer B0
    TB1CTL |= TBSSEL__ACLK;     // Select ACLK
    TB1CTL |= MC__UP;           // Using counting up mode
    TB1CCR0 = 6400;             // Set to count to 200ms

}


void delay(void) {

    TB1CCTL0 |= CCIE;               // Enable IRQ for Timer B0 CCR0
    TB1CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

    while(wait == 0) {}

    TB1CCTL0 &= ~CCIE;              // Disable TimerB0 IRQ
    TB1CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0



}

/*-------------------------------------------------------------------*/
/* Collect Sensor Measurement                                        */
/*-------------------------------------------------------------------*/
void sensor_measurement(void) {

    int i, j, k, m;
    m = 0;

    for(i=0; i < 25; i++) {

        P2IES &= ~BIT2;                 // LOW to HIGH sensitivity

        P2IE  |=  BIT2;                 // Enable IRQ for PWM
        P2IFG &= ~BIT2;                 // Clear flags for PWM

        while(start == 0){}

        P2IES |= BIT2;                  // HIGH to LOW sensitivity
        P2IFG &= ~BIT2;                 // Clear flags for PWM

        //TB0CTL |= TBCLR;
        TB0CCTL0 |= CCIE;               // Enable IRQ for Timer B0 CCR0
        TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

        while(start == 1){}            // Wait for PWM measurement

        P2IE  &= ~BIT2;                  // Disable IRQ for PWM
        P2IFG &= ~BIT2;                 // Clear flags for PWM

        TB0CCTL0 &= ~CCIE;              // Disable TimerB0 IRQ
        TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0


        if(cycles > 5 && m < 10) {

            measurement_array[m] = cycles / 60;        // Maybe 61
            m = m+1;

        }

        //measurement_array[i] = cycles / 58;

        // Convert number of cycles in pulse to a measurement
        cycles = 0;
        start = 0;
    }


    // Arranging the array from least to greatest


    int a, b, c;

    for (a=1; a < 10; ++a){
        b = measurement_array[a];
        for (c = a - 1; (c >= 0) && (b < measurement_array[c]); c--){
            measurement_array[c + 1] = measurement_array[c];
        }
        measurement_array[c + 1] = b;
    }

    /*
    // This code will return the mode of the array (most measurements that are equal should be the correct measurement)
    // If there is no mode, it returns the median (middle value of array that was sorted from least to greatest)
    i = 0; // Resetting variables
    */

    int duplicateCount = 0;
    int maxCount = 0;
    int prevCount = 0;
    int filter_value = 0;
    int median;

    a = 0;
    int x;
    int total_value = 0;

    while(a < 10){ // going through array
    prevCount=duplicateCount; // store previous count of duplicates
    duplicateCount=0;
        // counting duplicates
        while(measurement_array[a]==measurement_array[a+1]){
            duplicateCount++;
            a++;
        }
        // if # of duplicates is greater than the previous # or greater than max #
        // this count is the new mode
        if(duplicateCount>prevCount && duplicateCount>maxCount){
            filter_value=measurement_array[a];
            maxCount=duplicateCount;
            median=0;
        }
        // if the entry is not the same as another one, do nothing and continue
        else if(duplicateCount==0){
            a++;
        }
        else if(duplicateCount==maxCount){ //if the dataset has zero modes
            median=1;  //use the median of the dataset instead
        }
        // if no mode was found, filtered value is the median of the dataset
        if((filter_value == 0)||(median == 1)){
            //filter_value = measurement_array[sizeof(measurement_array)/2];
            //for(x=3; x<8; x++){
            //    total_value = total_value + measurement_array[x];
            //}
            //filter_value = total_value / 5;
            filter_value = measurement_array[5];
        }
    }
    sensor_value = filter_value;


}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // I/O Definitions (for testing)
    P1DIR |= BIT0;                  // Set P1.0 (LED1) as output
    P1OUT &= ~BIT0;                 // Start LED1 off

    init_sensor();                  // Initialize I/O settings for sensor
    init_sensorTimer();             // Initialize timer settings for PWM measurement
    init_startupTimer();

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O

    __enable_interrupt();           // Global IRQ enable

    delay();                        // Wait for sensor to power on

    while(1){

        // measures 10 times and sensor_value = final averaged value to transmit
        sensor_measurement();
    }

    return 0;
}


/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM I/O                                */
/*-------------------------------------------------------------------*/
#pragma vector = PORT2_VECTOR
__interrupt void ISR_PWM(void){


    if(start == 0) {

        TB0R = 0;
        start = 1;                  // Start measuring PWM
        TB0CCTL0 |= CCIS__VCC;

    } else if(start == 1) {

        cycles = TB0R;             // Collect TB0R value
        start = 0;
        TB0CCTL0 |= CCIS__GND;
    }

    P2IFG &= ~BIT2;                 // Clear flags for P2.2

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM Timer B0                           */
/*-------------------------------------------------------------------*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void){

    TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: Startup Timer B1                       */
/*-------------------------------------------------------------------*/
#pragma vector = TIMER1_B0_VECTOR
__interrupt void ISR_TB1_CCR0(void){

    wait = 1;
    TB1CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

}

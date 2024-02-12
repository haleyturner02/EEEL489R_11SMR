/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Sensor Setup and Testing                                          */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

volatile int cycles;
volatile int measurement;

/*-------------------------------------------------------------------*/
/* Sensor I/O Initialization                                         */
/*-------------------------------------------------------------------*/
void init_sensor(void) {
/*
    // PWM Pin Setup
    P2DIR &= ~BIT7;                 // Set P2.7 (PWM) as input
    P2REN |= BIT7;                  // Enable pull up/down resistors
    P2OUT |= BIT7;                  // Set as pull up resistor
    P2IES |= BIT7;                  // High to Low Sensitivity for PWM
   
    // Power Pin Setup
    P2DIR |= BIT6;                  // Set P2.6 (Power) as output
    P2OUT |= BIT6;                  // Start on (measuring)
*/

    // SW2 for testing
    P2DIR &= ~BIT3;
    P2REN |= BIT3;
    P2OUT |= BIT3;

    P2IES |= BIT3;
}

/*-------------------------------------------------------------------*/
/* TimerB0 Initialization                                            */
/*-------------------------------------------------------------------*/
void init_timer(void) {

    TB0CTL |= TBCLR;            // Clear Timer B0
    TB0CTL |= TBSSEL__ACLK;     // Select ACLK
    TB0CTL |= MC__UP;           // Using counting up mode
    TB0CCR0 = 1421;
    
}

/*-------------------------------------------------------------------*/
/* Collect Sensor Measurement                                        */
/*-------------------------------------------------------------------*/
void sensor_measurement(void) {

    //P2IE |= BIT7;                   // Enable IRQ for PWM
    //P2IFG &= ~BIT7;                 // Clear flags for PWM

    P2IE |= BIT3;                    // SW2 for testing 
    P2IFG &= ~BIT3;

    TB0CCTL0 |= CCIE;               // Enable IRQ for Timer B0 CCR0
    TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

    while(cycles == 0){}            // Wait for PWM measurement

    //P2IE &= ~BIT7;                   // Disable IRQ for PWM
    //P2IFG &= ~BIT7;                 // Clear flags for PWM

    P2IE &= ~BIT3;                  // SW2 for testing 
    P2IFG &= ~BIT3;

    TB0CCTL0 &= ~CCIE;              // Disable TimerB0 IRQ
    TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

    // Convert number of cycles in pulse to a measurement
    cycles = 0;

}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // I/O Defintiions (for testing)
    P1DIR |= BIT0;                  // Set P1.0 (LED1) as output
    P1OUT &= ~BIT0;                 // Start LED1 off

    init_sensor();                  // Initialize I/O settings for sensor
    init_timer();                   // Initialize timer settings for PWM measurement

    PM5CTL0 &= ~LOCKLPM5;           // Enable digital I/O

    __enable_interrupt();           // Global IRQ enable

    while(1){

       sensor_measurement();

    }

    return 0;
}


/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM I/O                                */
/*-------------------------------------------------------------------*/
#pragma vector = PORT2_VECTOR
__interrupt void ISR_PWM(void){

    cycles = TB0R;                  // Collect TB0R value
    P2IFG &= ~BIT3;                 // SW2 for testing 
    //P2IFG &= ~BIT7;                 // Clear flags for P2.7

}

/*-------------------------------------------------------------------*/
/* Interrupt Service Routine: PWM Timer B0                           */
/*-------------------------------------------------------------------*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void){

    P1OUT ^= BIT0;                  // Turn on LED1
    TB0CCTL0 &= ~CCIFG;             // Clear flags for Timer B0 CCR0

}

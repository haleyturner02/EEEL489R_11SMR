/*-------------------------------------------------------------------*/
/* EELE 489R - Electrical Engineering Design II                      */
/* Snowfall Measurement and Reporting Team                           */
/* Snowfall Computation Testing                                      */
/*-------------------------------------------------------------------*/

#include <msp430.h> 

/*-------------------------------------------------------------------*/
/* Variable Definitions for Storing Measurements and Time            */
/*-------------------------------------------------------------------*/
unsigned int u1 = 0;                                    // Counter for UART1

/* Measurement Global Variables */
char measurement1[] = "0000";                           // Newest measurement
char measurement2[] = "0000";                           // Measurement from 15 minutes prior
char measurement3[] = "0000";                           // Measurement from 30 minutes prior
char measurement4[] = "0000";                           // Measurement from 45 minutes prior

int calibrationDistance[] = "0000";
int calibrationButton = 0; // this turns to 1 if the button is pressed
int firstMeasurement = 1; // regardless of weather we have the calibration button we need the first measurement for a reference
int sensorMeasurement = 0;

/*-------------------------------------------------------------------*/
/* Snowfall Computation                                              */
/*-------------------------------------------------------------------*/
void snowfallCompute(void){
    int prevDistance;

    if (firstMeasurement == 1 || calibrationButton == 1){
        calibrationDistance = sensorMeasurement;
        measurement1 = char(calibrationDistance); // the first measurement just gets sent

        //reset variables
        calibrationButton = 0;
        firstMeasurement = 0;
    }
    else{
        prevDistance = int(measurement2); // now we can compare to the last measurement 
        snowfall = prevDistance - sensorMeasurement;
        measurement1 = snowfall;
    }

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

    PM5CTL0 &= ~LOCKLPM5;                               // Enable digital I/O
    UCB0CTLW0 &= ~UCSWRST;                              // Take out of software reset for I2C

    /* Enable interrupts */
    UCB0IE |= UCTXIE0 | UCRXIE0;;                       // Local interrupt enable for I2C TX0

    P3IE |= BIT0;                                       // Enable IRQ for SQW
    P3IFG &= ~BIT0;                                     // Clear flags for SQW

    __enable_interrupt();                               // Global IRQ enable

    clearAlarm();                                       // Start with Alarm 2 flag cleared in RTC Control Register

    while(1){                                           // Wait for interrupt to occur

        snowfallCompute();

    }

    return 0;
}

## Parent Device
The Parent Device is responsible for collecting measurements from Children, as well as its own sensor. Then, using the temperature sensor, it will compute the data for snowfall totals. Using this information, it will decide if new snowfall has occurred and transmit this information back to a base station. 

## Programming

Software Platform: Code Composer Studio
Code: parent.c
Required Hardware: MSP430FR2355 Launchpad (LP) + USB Cable, MSP430FR2355 Microcontroller, Jumper Wires
Connections:
Overview:
This code is what is run on the microcontroller on the Parent PCB/Device during normal operation for collecting and communicating data. The Launchpad is used to program the target microcontroller.

# Parent Device
The Parent Device is responsible for collecting measurements from Children Devices, as well as its own sensor. Then, using previous values, as well as a temperature sensor, it will compute the data for snowfall totals. Using this information, it will decide if new snowfall has occurred and transmit this information back to a base station. 


## Programming

**Software Platform:** Code Composer Studio \
**Code:** main.c \
**Required Hardware:** Parent PCB, MSP430FR2355 Launchpad (LP) + USB Cable, Jumper Wires \
**Connections:** SWD Interface \
**Overview:** \
This code is what is run on the microcontroller of the Parent Device during normal operation for collecting and communicating data. The Launchpad is used to program and debug the target microcontroller.

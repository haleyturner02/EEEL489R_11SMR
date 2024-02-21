# Child Devices
Each Child Device is responsible for collecting a sensor measurement when indicated by the Parent Device. This measurement will then be returned via the BLE modules back to the Parent Device within the localized area. 


## Programming

**Software Platform:** Code Composer Studio \
**Code:** child.c \
**Required Hardware:** MSP430FR2355 Launchpad (LP) + USB Cable, MSP430FR2310 Microcontroller, Jumper Wires \
**Connections:** \
**Overview:** \
This code is what is run on the microcontroller on the Child PCB/Device during normal operation for collecting and communicating data. The Launchpad is used to program the target microcontroller.

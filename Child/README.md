# Child Devices
Each Child Device is responsible for collecting a sensor measurement when indicated by the Parent Device. This measurement will then be returned via the BLE modules back to the Parent Device within the localized area. 


## Programming

**Software Platform:** Code Composer Studio \
**Code:** child.c \
**Required Hardware:** Child PCB, MSP430FR2355 Launchpad (LP) + USB Cable, Jumper Wires \
**Connections:** SWD Interface \
**Overview:** \
This code is what is run on the microcontroller of the Child Device during normal operation for collecting and communicating data. The Launchpad is used to program and debug the target microcontroller.

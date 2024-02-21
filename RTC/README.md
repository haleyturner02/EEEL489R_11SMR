# DS3232 Real Time Clock
The RTC will be programmed and debugged using an Arduino to configure its time and alarm settings. Once programmed, the RTC will alert the Parent Device every 15 minutes and communicate the current time via I2C. The Parent Device main code will contain an interrupt and processes for handling this communication, with the MCU acting as a master.


## Programming

**Software Platform:** Code Composer Studio \
**Code:** RTCSetup.c \
**Required Hardware:** MSP430FR2355 Launchpad (LP) + USB Cable, DS3231 RTC, Jumper Wires \
**Connections:** \
- +3.3V (LP) to VCC (RTC) 
- GND (LP) to GND (RTC) 
- P1.3 (LP) to SCL (RTC) 
- P1.2 (LP) to SDA (RTC) 
**Overview:** \
This code is used to initially setup the RTC by writing to its registers via I2C from the Launchpad. Each register stores a different time/date value (refer to Table), as well as settings for Alarm 1 and 2 and the control & status registers. 

**Register Map:**

| Register # | Purpose | Address | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|------------|---------|---------|-------|-------|-------|-------|-------|-------|-------|-------|
|      0     | Seconds |  0x00   |   0   | Tens  | Tens  | Tens  | Ones  | Ones  | Ones  | Ones  |
|      1     | Minutes |  0x01   |   0   | Tens  | Tens  | Tens  | Ones  | Ones  | Ones  | Ones  |
|      2     | Hours   |  0x02   |   0   | 12/24 | 20 hr | 10 hr | Ones  | Ones  | Ones  | Ones  |
|      4     | Date    |  0x04   |   0   |   0   | Tens  | Tens  | Ones  | Ones  | Ones  | Ones  |
|      5     | Month   |  0x05   |   0   |   0   |   0   | Tens  | Ones  | Ones  | Ones  | Ones  |
|      6     | Year    |  0x06   | Tens  | Tens  | Tens  | Tens  | Ones  | Ones  | Ones  | Ones  |

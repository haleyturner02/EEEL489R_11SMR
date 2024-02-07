# DS3232 Real Time Clock
The RTC will be programmed and debugged using an Arduino to configure its time and alarm settings. Once programmed, the RTC will alert the Parent Device every 15 minutes and communicate the current time via I2C. The Parent Device main code will contain an interrupt and processes for handling this communication, with the MCU acting as a master.

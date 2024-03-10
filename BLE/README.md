# DA14531MOD Bluetooth Low-Energy Modules 
The BLE modules will be programmed and debugged using SWD interface from an external processeser on EK-RA2E1 Development Board. Once programmed, BLE modules will communicate with each device's main code (Parent/Child) via UART1 on the MCUs. 

Note: ble_app_peripheral project from SDK 6.0.16 used as starting project for Child BLE Modules, only modified files are included in the Peripheral directory

Note: empty_peripheral_template project from SDK 6.0.16 used as starting project for Parent BLE Module, only modified files are included in Central directory

Note: custom application files (including UART configuration and implementation for communication to MCU) are included in the current directory (BLE)

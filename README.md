# WCH CH32V307 LwIP

## Overview

This repository contains a LwIP and FreeRTOS implementation for the WCH CH32V307 MCU.

The LwIP Implementation is based on the Ethernet code in the official WCH repository [WCH CH32V307V NetLib](https://github.com/openwch/ch32v307/tree/main/EVT/EXAM/ETH/NetLib). The LwIP itself is custom and uses LwIP 2.2.0d 

The FreeRTOS Implementation is based on the example in the official WCH repository [WCH CH32V307V FreeRTOS](https://github.com/openwch/ch32v307/tree/main/EVT/EXAM/FreeRTOS/FreeRTOS).

This project doesn't use the Mounriver IDE. It uses the Mounriver Toolchain within a Makefile environment.
   
### Build and Flash
* Build with `make`
* Flash with `make flash`
  
### Usage
The IP Address is configured over DHCP. The LwIP httpd service is available on Port 80.

### Debugging
* Run openocd with `make gdb`

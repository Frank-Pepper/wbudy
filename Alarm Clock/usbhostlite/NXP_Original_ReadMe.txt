*     NXP USB Host Stack
*
*     (c) Copyright 2009, NXP SemiConductors
*     (c) Copyright 2009, OnChip  Technologies LLC
*     All Rights Reserved
*
*     http://www.nxp.com/microcontrollers
*     http://www.onchiptech.com
*
*     History
*     2009.07.14  ver 0.01    Preliminary version, first Release 
*
-------------------------------------------------------------------------------------------------------------------
				             USB Host Sample for LPC17xx 
-------------------------------------------------------------------------------------------------------------------

1. What is this sample ?

   This sample demonstrates how to access the files on "USB Flash drive" connected to USB Host port on Keil
   MCB1700 board using NXP Semiconductors LPC17xx MCU. 

   When you connect a "USB Flash Drive" to the USB host port on MCB1700 board, the USB 
   host sample mounts the file system on the "USB Flash Drive". 

2. How to setup the sample ?

   To run the mass storage sample, you need the following.

      . Keil MCB1700 board
      . Keil ULink USB-JTAG adapter for flash programming.
      . Keil uVision3 IDE to open the project.
      . Serial cable to see the log messages on the Hyper Terminal.

   Jumper Settings of the MCB1700 board:

   USB as Host
   Two jumpers closed on silk screen marked as "HOST", middle position.

3. How to see the log messages on Hyper Terminal ?

   Connect a serial cable to UART #0 on the MCB1700 board.
   Configure the Hyper Terminal settings as 115200-N-8-1 and "No flow control".

   Leave both jumpers "ISP" and "RST" open near the "RESET" and "INT0" push buttons.

   Note:  The following macros must be defined in the usbhost_lpc17xx.h file.
   #define  PRINT_Log       UART_Printf
   #define  PRINT_Err(rc)   UART_Printf("ERROR: In %s at Line %u - rc = %d\n", __FUNCTION__, __LINE__, rc)

4. The user is provided with three sample functions, Main_Read(), Main_Write() and Main_Copy() defined in usbhost_main.c.

	If the user calls Main_Read(), the host sample reads data from file "FILENAME_R" into a buffer.
	If the user calls Main_Write(), the host sample writes data to file  "FILENAME_W" from a buffer.
	If the user calls Main_Copy(), the host sample copies data from file "FILENAME_R" to file "FILENAME_W".

	Note: The file name defined by macro "FILENAME_R"  must be present on the device (usb flash drive).

5. How to download and run the demo ?

   1. Connect the power supply by using USB or optional power supply.

   2. Connect the Keil U-Link USB J-Tag

   3. Connect the serial cable at UART#0 to see the log messages.

   4. Open the hyper terminal and make settings as shown in Hyper terminal settings.

   5. Open the USBHostLite directory.

   6. Open the project UsbHostLite.Uv2 by double clicking on it.

   7. Goto project tab and click 'Rebuild all target files' to compile the project.

   8. Goto Flash tab and click 'Download'.

   9. Connect a USB flash drive at USB host port.


Disclaimer
==========
This software is provided AS-IS by NXP Semiconductors.  NXP Semiconductors 
does not support or warrant the software contained herein for any purpose 
other than for informational use.  Please contact OnChip Technologies LLC 
for further assistance including support options and other embedded USB 
host/device/OTG stacks.  

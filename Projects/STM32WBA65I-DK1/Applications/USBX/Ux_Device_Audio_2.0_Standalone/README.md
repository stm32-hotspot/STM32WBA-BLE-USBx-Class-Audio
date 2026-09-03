# Ux_Device_Audio_2.0_Standalone

The objective of this project is to provide examples of the implementation of the USB Class Audio using the Azure RTOS USBX stack on the STM32WBA6 MCU.

The Ux_Device_Audio_2.0_Standalone application streams audio from USB to 3.5mm jack output.
<br>

### __Keywords__

Connectivity, Bare Metal, USB, USB Class Audio

### __Hardware and Software environment__

This example runs on **STM32WBA65I-DK1** boards.

To build the projects, you need one of the following IDE:
  - IAR Embedded Workbench for ARM (EWARM) 9.60.3
  - STM32CubeIDE 2.2.0

### __How to use it?__
The following materials are needed to replicate the demo:
  - An **STM32WBA65I-DK1** running this project
  - A USB Class Audio host: any PC or smartphone
  - A USB cable to connect the Discovery Kit to the USB host (usually a USB-A to USB-C cable for PC and USB-C to USB-C for smartphone)
  - Headphones with a 3.5mm jack input


The following diagram details how to set up the demo:
<div align="left">
    <img src="../../../../../Utilities/Media/Setup_Ux_Device_Audio_2.0_Standalone.png" alt="Ux_Device_Audio_2.0_Standalone setup" width="800" height="auto">
</div>

To power the Discovery Kit from the USB and not the ST-Link, move the jumper to "5V_USB_MCU":
<div align="left">
    <img src="../../../../../Utilities/Media/Setup_Jumper_USB.png" alt="Set Jumper on 5V_USB_MCU" width="500" height="auto">
</div>

### __Documentation__

   - Wiki page related to the USBX implementation on STM32 is available here:
     - <a href="https://wiki.st.com/stm32mcu/wiki/Introduction_to_USBX"> Introduction to USBX</a>
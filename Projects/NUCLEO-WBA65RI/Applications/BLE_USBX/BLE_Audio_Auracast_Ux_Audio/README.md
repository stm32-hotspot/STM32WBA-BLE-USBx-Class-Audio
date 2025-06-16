# BLE_Audio_Auracast_Ux_Audio

The objective of this project is to provide examples of the implementation of the USB Class Audio using the Azure RTOS USBX stack on the STM32WBA6 MCU.

The BLE_Audio_Auracast_Ux_Audio application streams audio from USB to Auracast stream.
<br>

### __Keywords__

Connectivity, BLE, BLE protocol, BLE Audio, Bare Metal, Auracast, USB, USB Class Audio

### __Hardware and Software environment__

This example runs on **NUCLEO-WBA65RI** boards.

To build the projects, you need one of the following IDE:
  - IAR Embedded Workbench for ARM (EWARM) 9.20.1
  - STM32CubeIDE 1.18.1

> [!WARNING]
> Using STM32CubeIDE in **Debug** configuration can cause audio glitches due to GCC optimization for this debug. Consider using the **Release** configuration when not debugging, or use IAR.

### __How to use it?__
The following materials are needed to replicate the demo:
  - A **NUCLEO-WBA65RI** running this project
  - An **STM32WBA55-DK1** or an **STM32WBA65I-DK1** running the BLE_Audio_PBP_Sink project available in the [STM32CubeWBA firmware package 1.6.1](https://www.st.com/en/embedded-software/stm32cubewba.html)
  - A USB Class Audio host: any PC or smartphone
  - A USB cable to connect the Discovery Kit to the USB host (usually a USB-A to USB-C cable for PC and USB-C to USB-C for smartphone)
  - Headphones with a 3.5mm jack input


The following diagram details how to set up the demo:
<div align="left">
    <img src="../../../../../Utilities/Media/Setup_BLE_Audio_Auracast_Ux_Audio.png" alt="BLE_Audio_Auracast_Ux_Audio setup" width="800" height="auto">
</div>

To power the Discovery Kit from the USB and not the ST-Link, move the jumper to "5V_USB_MCU":
<div align="left">
    <img src="../../../../../Utilities/Media/Setup_Jumper_USB.png" alt="Set Jumper on 5V_USB_MCU" width="500" height="auto">
</div>

### __Documentation__

   - Wiki pages related to the LE Audio solutions developped by STMicroelectronics are available here:
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Introduction_to_Bluetooth_LE_Audio"> Introduction to Bluetooth® Low Energy Audio</a>
	 - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_LC3_Codec"> Bluetooth® Low Energy audio - STM32WBA LC3 codec and audio data path</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Architecture_and_Integration"> Bluetooth® Low Energy audio - STM32WBA Architecture and Integration</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_Content_Control"> Bluetooth® Low Energy audio - Content Control</a>
     - <a href="https://wiki.st.com/stm32mcu/wiki/Connectivity:Bluetooth_LE_Audio_-_STM32WBA_Public_Broadcast_Profile"> Bluetooth® Low Energy audio - STM32WBA Public broadcast profile</a>

   - Wiki page related to the USBX implementation on STM32 is available here:
     - <a href="https://wiki.st.com/stm32mcu/wiki/Introduction_to_USBX"> Introduction to USBX</a>


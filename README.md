# Toyota RAV4 V (XA50) 2019+ climate control panel hacking (STM32G474RE)
An STM32 HAL example of communicating over the LIN bus with the climatronic panel.

# Motivation
If you are familiar with [Ford Mustang VI (S550) 2015+ instrument panel cluster hacking](https://github.com/ufnalski/ford_mustang_cluster_h503rb), you already know the drill :slightly_smiling_face: Engaging classrooms/labs. Hands-on training on the LIN bus communication. We need at least two nodes to play with the bus. The LIN bus has the master-slave architecture. A slave node built from scratch using a Nucleo board, a LIN transceiver, a display, a couple of push buttons plus rotary encoder knobs (to let your trainees interact with some common components of such HMIs[^1]), and a breadboard with a power supply would cost you around 30 EUR. For the same money you can play with a state-of-the-art control panel featuring two encoder knobs, several push buttons with LED indicators, a display, and a dimmable backlight. It's gonna be fun to turn all that stuff alive and interact with it. Using such training aids is definitely fun for me and I hope that this fun can be contagious. And you can learn a lot interacting with production devices, e.g. that complying with the standard is essential if you want the LIN device to respond to a master device we are going to build here.

[^1]: Human-machine interface

![Toyota RAV4 V (XA50) 2022 climate control panel](/Assets/Images/toyota_rav4_university_lab.jpg)

# How to deal with car modules that use LIN bus to communicate
Similarly to the car modules that use CAN bus to communicate, probably the most convenient way would be to get your hands on a working car and sniff the bus. If this is not the case for you, as was not the case for me with Toyota RAV4, still we have some options.

Option 1. Choose a module that has been hacked by other hobbyists. And by hacking I don't mean here any illegal activity. I mean completely legal reverse engineering done by hobbyists who enjoy e.g. adding features and extending functionality of their privately owned cars. Now you can focus on porting their projects to your favorite STM32 uC.

Option 2. LIN uses 6-bit identifiers. It is then fairly easy (not very time consuming) to trigger the response of the device such as HMI. Use a bus sniffer to catch the response and then to observe how the response changes with pressed buttons or turned knobs. Mission accomplished :sunglasses: Beware of completely undocumented devices that have to be configured/initialized over the bus before they enter their normal mode of operation, e.g. before they start measuring a distance. The brute force method to find the proper configuration sequence would be impractical in most cases. Modules that often do not need any initialization sequence over the LIN interface and thus are ready to visibly act on commands/requests immediately after powering them up are:
* wiper actuators,
* window winders,
* automated charging flaps,
* HVAC actuators (to position the flaps of the air conditioning system),
* automatic folding side view mirrors,
* switch and rotary knob panels,
* indicator and display panels.

# Why Toyota RAV4 V (XA50) climatronic panel?
Because of [bitzero.tech](https://bitzero.tech/). Thank you, Jacob Schooley :exclamation: 

# Missing files?
Don't worry :slightly_smiling_face: Just hit Alt-K to generate /Drivers/CMCIS/ and /Drivers/STM32G4xx_HAL_Driver/ based on the .ioc file. After a couple of seconds your project will be ready for building.

# Tools
Reverse engineering communication without a bus sniffer or at least a protocol decoder? Do not waste your precious time. Here are the tools I use at the home lab:
* [LUC](https://ucandevices.github.io/ulc.html) plus [uCCBViewer](https://github.com/UsbCANConverter-UCCbasic/uCCBViewer/releases) (approx. 20 EUR),
* [DSLogic Plus](https://www.dreamsourcelab.com/product/dslogic-series/)[^2] plus [DSView](https://www.dreamsourcelab.com/download/),

and the one I enjoy at the university:
* [PLIN-USB](https://www.peak-system.com/PLIN-USB.485.0.html?&L=1) plus [PLIN-View Pro](https://www.peak-system.com/PLIN-View-Pro.243.0.html?&L=1).

[^2]: Any logic analyzer will do the job, even the one for 10 EUR, if compatible with a software that offers a LIN bus decoder, e.g. [Logic 2](https://www.saleae.com/pages/downloads), [DSView](https://www.dreamsourcelab.com/download/) or [KingstVIS](https://www.qdkingst.com/en/download). The max data transfer speed allowed by the LIN standard is just 20 kBaud.

# Wiring diagram
![Toyota RAV4 V (XA50) System Wiring Diagrams - Climatronic panel](/Assets/Images/toyota_rav4_genV_air_conditioning_control_panel.JPG)

For more visit [/Assets/Images/](/Assets/Images/).

# If you are new to the STM32 UART/LIN[^3] internal peripheral and the LIN protocol
* [LIN Bus Explained - A Simple Intro](https://www.csselectronics.com/pages/lin-bus-protocol-intro-basics) (CSS Electronics)
* [Introduction to the LIN bus](https://kvaser.com/about-can/can-standards/linbus/) (Kvaser)
* [LIN Bus Simplified: The Ultimate Guide in 2024](https://www.autopi.io/blog/lin-bus-protocol-explained/) (AutoPi)
* [LIN Overview](https://developerhelp.microchip.com/xwiki/bin/view/applications/lin/overview/)
* [STM32 as the Lin Master || Configure & Send Data](https://controllerstech.com/stm32-uart-8-lin-protocol-part-1/) (ControllersTech)
* [Using Lin Transceivers to communicate between master and slave](https://controllerstech.com/stm32-uart-9-lin-protocol-part-2/) (ControllersTech)
* [Master – slave communication using LinBus](https://controllerstech.com/stm32-uart-10-lin-protocol-part-3/) (ControllersTech)
* [LIN Bus Basics for Beginners](https://lipowsky.com/downloads/Software/LIN-Basics_for_Beginners-EN.pdf)
* [LIN Specification Package Revision 2.2A](https://www.cs-group.de/wp-content/uploads/2016/11/LIN_Specification_Package_2.2A.pdf)

[^3]: The LIN bus uses the UART peripheral.

# Exemplary hardware for breadboarding
* [Dual LIN Click](https://www.mikroe.com/dual-lin-click) (Mikroe)
* [Gravity 130 DC motor module](https://wiki.dfrobot.com/Gravity__130_DC_Motor_SKU__DFR0411) (DFRobot)[^4]

[^4]: The fan is here just to have more fun.

# Sources and inspirations
* [Hacking My Car's Climate Controls: LIN Reverse Engineering](https://bitzero.tech/posts/2022/11/30/hacking-my-cars-climate-controls-lin-reverse-engineering)
* [Hacking My Car's Climate Controls, Part 2: Building the Interceptor](https://bitzero.tech/posts/2023/04/06/hacking-my-cars-climate-controls-part-2)
* [ToyotaLinInterceptor](https://github.com/jbschooley/ToyotaLinInterceptor)
* [eCarEdu=teoria+praktyka](https://ecaredu.pl/) (in Polish)

For a complete parser visit [/Assets/Parser/](/Assets/Parser/). Thank you, Mateusz Buraczewski :exclamation:

# Issues encountered and not solved
```c
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);
```
is never called if
```c
#define RX_TX_BUFFER_SIZE 12
```
does not match the length of the actual response, which seems to contradict the idea of receiving to idle
```c
HAL_UARTEx_ReceiveToIdle_DMA(&huart3, RxData, RX_TX_BUFFER_SIZE);
__HAL_DMA_DISABLE_IT(&hdma_usart3_rx, DMA_IT_HT);
```
My bet is that it is something related to UART configured as LIN, because an identical HAL code works perfectly fine for UART as the "regular" UART - tested on the NEO-6M-V2 GPS module.

BTW, a nice application note regarding STM32 UART DMA RX and TX is available [here](https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx).

# Call for action
Create your own [home laboratory/workshop/garage](http://ufnalski.edu.pl/control_engineering_for_hobbyists/2024_dzien_otwarty_we/Dzien_Otwarty_WE_2024_Control_Engineering_for_Hobbyists.pdf)! Get inspired by [ControllersTech](https://www.youtube.com/@ControllersTech), [DroneBot Workshop](https://www.youtube.com/@Dronebotworkshop), [Andreas Spiess](https://www.youtube.com/@AndreasSpiess), [GreatScott!](https://www.youtube.com/@greatscottlab), [ElectroBOOM](https://www.youtube.com/@ElectroBOOM), [Phil's Lab](https://www.youtube.com/@PhilsLab), [atomic14](https://www.youtube.com/@atomic14), [That Project](https://www.youtube.com/@ThatProject), [Paul McWhorter](https://www.youtube.com/@paulmcwhorter), and many other professional hobbyists sharing their awesome projects and tutorials! Shout-out/kudos to all of them!

> [!WARNING]
> Control engineering - try this at home :exclamation:

190+ challenges to start from: [Control Engineering for Hobbyists at the Warsaw University of Technology](http://ufnalski.edu.pl/control_engineering_for_hobbyists/Control_Engineering_for_Hobbyists_list_of_challenges.pdf).

Stay tuned :exclamation:

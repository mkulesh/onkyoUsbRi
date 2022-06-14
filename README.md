[![License](https://img.shields.io/badge/license-GNU_GPLv3-orange.svg)](https://github.com/mkulesh/onkyoUsbRi/blob/main/LICENSE)

# Onkyo Remote Control Interface: USB-Ri adapter based on STM32F303

Onkyo HiFi devices (players, amplifiers, receives) usually have so called RI port on the back panel.  It is a 3.5mm mono jack that can be used to connect other Onkyo elements. For example if we connect an Onkyo amplifier with an IR remote, the amplifier IR remote can control both the amplifier and the player thanks to the Remote Interactive port. In fact we can daisy chain several Onkyo elements (Tape player, DVD player, etc.) and control all the elements with a single remote.

The problem is that modern Onkyo players and receivers do not more support older devices like tape-decks or MD-players. The hardware still exist but such a support is removed from actual firmware. 

However, we can control older Onkyo devices from a PC using small USB-Ri adapter and appropriate software. This project contains PCB layout and firmware for such an adapter.

The  adapter is based on  STM32F303 MCU. USB interface is implemented using FT321XS IC. The device is powered from USB and contains a 3.5mm mono jack used to attach it to an Onkyo device.

## PCB

<img src="https://raw.githubusercontent.com/mkulesh/onkyoUsbRi/main/images/pcb.jpg" align="center" height="600">

## Adapter

<img src="https://raw.githubusercontent.com/mkulesh/onkyoUsbRi/main/images/device.jpg" align="center" height="600">

## Schematic

<img src="https://raw.githubusercontent.com/mkulesh/onkyoUsbRi/main/images/schematic.png" align="center" height="600">

## Desktop App

In order to control Onkyo via this adapter, [Enhanced Music Controller](https://github.com/mkulesh/onpc) shall be used:

<img src="https://raw.githubusercontent.com/mkulesh/onkyoUsbRi/main/images/app.png" align="center" height="600">

## License

This software is published under the *GNU General Public License, Version 3*

Copyright © 2022 by Mikhail Kulesh

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have
received a copy of the GNU General Public License along with this program.

If not, see [www.gnu.org/licenses](https://www.gnu.org/licenses).


# Lens Encoder Simulator

Firmware for a Nucleo-144 STM32L496ZGT board that imitates an [Indiemark Lens Encoder](https://www.loledvirtual.com/indiemark/).

This project is intended for development and testing when an original Indiemark Lens Encoder is not available.

## Verified Project Details

- Target MCU: STM32L496ZGTxP / STM32L496ZGT6P.
- USB device: Custom HID, product string `Lens Encoder Simulator`.
- USB VID/PID: `0x239A` / `0xCAFE`.
- Encoder input: EC11 rotary pulse encoder on the TIM3 encoder interface, using PE3/PE4.
- PE3 and PE4 use the MCU's internal pull-up resistors, so no external pull-up resistors or debounce capacitors are required for basic use.
- Firmware reports encoder position and status over USB HID.

## How to connect

Use the 3-pin side of the EC11 encoder:

- One side pin, for example left: connect to PE3.
- Middle pin: connect to GND.
- Other side pin, for example right: connect to PE4.


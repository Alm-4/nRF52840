# Bare-Metal nRF52 Wireless Messaging

This is a bare-metal nRF52840 application that uses the on-chip 2.4 GHz radio
to transmit and receive short text messages. It also provides a UART terminal
for entering messages and displaying received data. The radio is configured for
1 Mbit Nordic mode on channel 7 (2407 MHz), with a maximum payload of 127
bytes.

## Requirements

- nRF52840 development kit (two boards are useful for testing communication)
- GNU Make
- Arm GNU Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, and
  `arm-none-eabi-size`)
- SEGGER J-Link software for programming
- A serial terminal

The toolchain path is set by `TOOLCHAIN_BASE` in `Makefile`; update it if the
toolchain is installed elsewhere.

## Build

From this directory, run:

```sh
make
```

The build produces:

- `build/nrf52840_bare_metal.elf` — ELF image for debugging
- `build/nrf52840_bare_metal.bin` — raw firmware image for programming
- `build/nrf52840_bare_metal.map` — linker map

Remove generated files with:

```sh
make clean
```

## Program and run

`make` builds the firmware but does not flash the board. With a J-Link GDB
server running, connect GDB to the ELF:

```sh
JLinkGDBServer -select USB -device nRF52840_xxAA -endian little \
  -if SWD -noir -noLocalhostOnly -nologtofile -port 2331 \
  -SWOPort 2332 -TelnetPort 2333
```

In another terminal:

```sh
arm-none-eabi-gdb -ex "target remote localhost:2331" \
  build/nrf52840_bare_metal.elf
```

Then load and run it from the GDB prompt:

```gdb
monitor reset 0
load
monitor reset
continue
```

## Use the wireless console

Open the board's UART at **115200 baud, 8 data bits, no parity, 1 stop bit**.
The application displays a `$` prompt and echoes typed characters. Enter a
message followed by Enter:

1. The message is transmitted over the radio.
2. The board switches to receive mode and waits for a packet.
3. The received message is printed over UART.

For a loopback-style test, program two boards with this firmware and connect
each to a serial terminal. Type a message on either board; both boards use the
same radio address and channel, so the other board can receive it. Reception
is blocking, so the terminal may not respond until a packet arrives.

## Project layout

- `src/main.c` — clock, radio, UART, transmit, and receive logic
- `src/startup.c` — reset handler, vector table, and device initialization
- `src/nrf52840_xxaa.ld` — nRF52840 flash and RAM layout
- `Makefile` — cross-compilation and linking rules
- `build/` — generated build artifacts
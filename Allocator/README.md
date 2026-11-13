# Bare-Metal nRF52840 Memory Allocator

This project is a small bare-metal application for the Nordic nRF52840
microcontroller. It starts the device without an operating system, configures
UART0, and provides a serial console for exercising a simple 1 KiB heap
allocator.

## Requirements

- nRF52840 development kit
- GNU Make
- Arm GNU Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-objcopy`, and
  `arm-none-eabi-size`)
- SEGGER J-Link software for programming and debugging
- A serial terminal for UART communication

The toolchain location is configured in the `TOOLCHAIN_BASE` variable in
`Makefile`. Update it if the toolchain is installed somewhere else.

## Build

From this directory, run:

```sh
make
```

The build creates `build/nrf52840_bare_metal.elf` for debugging and
`build/nrf52840_bare_metal.bin` for programming. It also prints the ELF size.
To remove generated files, run:

```sh
make clean
```

## Program and debug

`make` only builds the firmware. With the nRF52840DK connected, start a J-Link
GDB server:

```sh
JLinkGDBServer -select USB -device nRF52840_xxAA -endian little \
  -if SWD -noir -noLocalhostOnly -nologtofile -port 2331 \
  -SWOPort 2332 -TelnetPort 2333
```

In another terminal, connect GDB to the generated ELF:

```sh
arm-none-eabi-gdb -ex "target remote localhost:2331" \
  build/nrf52840_bare_metal.elf
```

At the GDB prompt, load and run the firmware:

```gdb
monitor reset 0
load
monitor reset
continue
```

## UART console

Connect to the board's UART at **115200 baud, 8 data bits, no parity, 1 stop
bit**. The application displays a `$` prompt and supports:

```text
alloc <bytes>       Allocate memory and print its address
free <address>      Free a previously allocated block
status              List free blocks
```

Enter commands one line at a time. Allocation uses a 1 KiB memory pool; the
allocator requires valid addresses when freeing blocks.

## Project layout

- `src/main.c` — UART console and command handling
- `src/myalloc.c`, `src/myalloc.h` — fixed-size heap allocator
- `src/startup.c` — vector table, reset handler, and device initialization
- `src/nrf52840_xxaa.ld` — flash and RAM layout
- `Makefile` — cross-compilation and linking rules
- `build/` — generated object, ELF, BIN, and map files
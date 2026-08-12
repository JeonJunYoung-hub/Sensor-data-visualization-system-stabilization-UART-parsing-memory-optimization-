# UART Communication Troubleshooting

> A **DRI** side project.

Case study and cleaned example code for debugging UART communication between an Arduino sensor controller and a CircuitPython LED matrix display controller.

The project focuses on a common embedded failure mode: hardware is working, serial data is being transmitted, but the receiving display stays blank because the software parses partial UART packets too early.

## Repository Layout

- `arduino/dual_sen55_uart_sender/` - Arduino sketch that reads two SEN55 sensors through a TCA9548A I2C multiplexer and transmits a line-based UART packet.
- `circuitpython/metro_m4_led_matrix_uart/` - CircuitPython receiver for a Metro M4 RGB matrix setup. It buffers UART bytes until newline-delimited packets are complete before drawing text.
- `docs/troubleshooting-case-study.md` - Root cause analysis and debugging notes.
- `images/` - LED panel result photos.

## Interfaces used

**UART · I2C**

| Interface | What it carries |
|---|---|
| UART | Newline-delimited packets, Arduino sender → CircuitPython Metro M4 receiver |
| I2C | Two Sensirion SEN55 sensors behind a TCA9548A multiplexer |

## Communication Contract

The sender emits newline-terminated packets:

```text
0,DEVICE001,A,<sensor A values>,B,<sensor B values>
```

The receiver:

1. reads UART bytes in small chunks,
2. appends decoded bytes into a persistent receive buffer,
3. waits until `\n` is present,
4. parses only complete lines,
5. renders packet text onto the LED matrix.

## Lessons Learned

- UART `read()` can return partial frames.
- Comma splitting raw chunks is unreliable unless framing is handled first.
- Matching baud rates is necessary but not sufficient.
- A newline-delimited protocol makes the receiver deterministic and much easier to debug.
- Print raw bytes during bring-up before debugging the display layer.

## Hardware Context

- Arduino-compatible sensor MCU
- CircuitPython Metro M4 style display controller
- RGB Matrix Shield and chained 64x32 LED panels
- Two SEN55 sensors on a TCA9548A I2C multiplexer

## Results

![LED matrix result 1](images/LED1.jpg)
![LED matrix result 2](images/LED2.jpg)

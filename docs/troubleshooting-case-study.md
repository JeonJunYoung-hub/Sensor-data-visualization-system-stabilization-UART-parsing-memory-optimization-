# Troubleshooting Case Study

## Initial Symptom

- LED panels powered on but displayed nothing.
- UART transmission appeared active.
- Display test code worked, but live sensor text did not render.

## Initial Hypotheses

1. Missing common ground
2. Voltage instability
3. Logic-level mismatch
4. RGB matrix shield configuration issue
5. Faulty LED panels
6. Serial baud-rate mismatch

## Hardware Verification

- TX voltage measured near 3.3 V and was stable.
- Supply voltage drop was observed through the breadboard wiring but did not explain the blank display by itself.
- Ground continuity was verified.
- Metro M4, RGB Matrix Shield, and LED panels were confirmed functional with independent display test code.
- Sender and receiver were both configured for 115200 baud.

Conclusion: the failure was in UART receive/parsing logic, not the LED panels.

## Root Cause

The original receiver treated each `uart.read()` result as if it contained one complete packet. In practice, UART is a byte stream. A read can return:

- half of one packet,
- one full packet,
- one packet plus the start of the next packet,
- or only a few bytes depending on timing.

Parsing comma-separated fields before reconstructing full newline-delimited lines caused incomplete display payloads and blank rendering.

## Fix

The CircuitPython receiver keeps a persistent string buffer:

```python
recv_buffer += data.decode("utf-8")

while "\n" in recv_buffer:
    line, recv_buffer = recv_buffer.split("\n", 1)
    line = line.strip()
```

Only complete lines are passed into the display renderer.

## Additional Bug Found

The original receiver source had an accidental stray `q` inside the decode block. That made the script syntactically invalid. The cleaned version removes it.

## Debugging Checklist

- Confirm both sides use the same baud rate.
- Print raw received bytes before parsing.
- Add a packet prefix, such as `0,`, to identify packet types.
- Add a line terminator from the sender.
- Buffer incoming bytes until a full line is available.
- Keep display rendering separate from UART parsing.
- Test the display with fixed text before connecting the sensor sender.

## Final Packet Strategy

The sender emits compact newline-terminated packets:

```text
0,DEVICE001,A,<temp,rh,pm1,pm25,pm4,pm10,voc,nox,co>,B,<same fields>
```

The receiver formats the packet into fixed-width segments for a two-panel 64x32 LED matrix chain.

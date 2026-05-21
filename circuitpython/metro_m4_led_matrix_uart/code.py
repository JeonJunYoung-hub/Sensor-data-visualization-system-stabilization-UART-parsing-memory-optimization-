import time

import board
import busio
import displayio
import framebufferio
import rgbmatrix
import terminalio
from adafruit_bitmap_font import bitmap_font
from adafruit_display_text import label


displayio.release_displays()

BASE_WIDTH = 64
BASE_HEIGHT = 32
CHAIN_ACROSS = 2
TILE_DOWN = 1
TEXT_COLOR = 0xA0A8EB

matrix = rgbmatrix.RGBMatrix(
    width=BASE_WIDTH * CHAIN_ACROSS,
    height=BASE_HEIGHT * TILE_DOWN,
    bit_depth=1,
    rgb_pins=[board.D2, board.D3, board.D4, board.D5, board.D6, board.D7],
    addr_pins=[board.A0, board.A1, board.A2, board.A3],
    clock_pin=board.A4,
    latch_pin=board.D10,
    output_enable_pin=board.D9,
)

display = framebufferio.FramebufferDisplay(matrix, auto_refresh=False)
uart = busio.UART(board.TX, board.RX, baudrate=115200, timeout=0.5)
recv_buffer = ""


def draw_packet_type_0(text):
    group = displayio.Group()

    for i in range(6):
        segment = text[i * 10:(i + 1) * 10]
        line = label.Label(
            terminalio.FONT,
            color=TEXT_COLOR,
            scale=1,
            text=segment,
        )
        line.x = 2 if i < 3 else 66
        line.y = 5 + (i % 3) * 11
        group.append(line)

    display.root_group = group
    display.refresh(minimum_frames_per_second=1)


def draw_packet_type_1(text):
    font = bitmap_font.load_font("fonts/Arial-12.bdf")
    group = displayio.Group()

    for i in range(4):
        segment = text[i * 7:(i + 1) * 7]
        line = label.Label(
            font=font,
            color=TEXT_COLOR,
            scale=1,
            text=segment,
        )
        line.x = 2 if i < 2 else 66
        line.y = 6 if i % 2 == 0 else 22
        group.append(line)

    display.root_group = group
    display.refresh(minimum_frames_per_second=1)


def draw_packet_type_2(text):
    font = bitmap_font.load_font("fonts/Arial-14.bdf")
    group = displayio.Group()

    segments = [
        (terminalio.FONT, text[0:10], 2, 5),
        (font, text[10:16], 2, 21),
        (terminalio.FONT, text[16:26], 66, 5),
        (font, text[26:32], 66, 21),
    ]

    for font_obj, segment, x, y in segments:
        line = label.Label(
            font_obj,
            color=TEXT_COLOR,
            scale=1,
            text=segment,
        )
        line.x = x
        line.y = y
        group.append(line)

    display.root_group = group
    display.refresh(minimum_frames_per_second=1)


def handle_line(line):
    print("Received:", line)

    if line.startswith("0,"):
        draw_packet_type_0(line[2:])
    elif line.startswith("1,"):
        draw_packet_type_1(line[2:])
    elif line.startswith("2,"):
        draw_packet_type_2(line[2:])
    else:
        print("Unknown packet type")


while True:
    data = uart.read(128)

    if data:
        print("Raw data:", data)
        try:
            recv_buffer += data.decode("utf-8")
        except UnicodeError:
            print("Decode error")
            recv_buffer = ""

        while "\n" in recv_buffer:
            line, recv_buffer = recv_buffer.split("\n", 1)
            line = line.strip()
            if line:
                handle_line(line)

    time.sleep(0.05)

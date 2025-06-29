# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Single Color RGB565 Blob Tracking Example
#
# This example shows off single color RGB565 tracking using the OpenMV Cam.

import sensor
import time
import math
from pyb import UART



threshold_index = 0  # 0 for red, 1 for green, 2 for blue

# Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
# The below thresholds track in general red/green/blue things. You may wish to tune them...
thresholds = [(72, 100, 2, 127, -4, 127)]  # generic_blue_thresholds

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
sensor.set_auto_gain(False)  # must be turned off for color tracking
sensor.set_auto_whitebal(False)  # must be turned off for color tracking
sensor.set_brightness(-3)
sensor.set_saturation(-3)
uart = UART(3, 115200, timeout_char=200)
clock = time.clock()
num1 = 20
num2 = 30
while True:
    if uart.any():
        string = uart.readline()
        #if string == "@DEVICE_A_REC?":

        uart.write(f"@DEVICE_A_RECEIVED:{num1},{num2}\n")
    clock.tick()
    img = sensor.snapshot()
    num1 = 20
    num2 = 30
    for blob in img.find_blobs(
        [thresholds[threshold_index]],
        pixels_threshold=200,
        area_threshold=200,
        merge=True,
    ):
        # These values depend on the blob not being circular - otherwise they will be shaky.
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
        # Note - the blob rotation is unique to 0-180 only.
        img.draw_keypoints(
            [(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20
        )
        num1 = 70
        num2 = 100

    print(clock.fps())

#!/bin/python3

import sys
import time
from datetime import datetime
import os


increment = 0.1
sleep_time = 5
while True:
    time.sleep(sleep_time)
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    msg = f"sending an error message at {current_time}\n"
    os.write(2, msg.encode())
    time.sleep(sleep_time)
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    msg = f"sending an info message at {current_time}\n"
    os.write(1, msg.encode())

    sleep_time += increment
    increment += increment
    increment = min(increment, 3600)
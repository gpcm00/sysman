#!/bin/python3

import random
import sys
import time
import os
from datetime import datetime


min_wait = int(sys.argv[1])
max_wait = int(sys.argv[2])

current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
msg = f"Initializing die_hard at {current_time}\n"
os.write(1, msg.encode())

wait_time = random.randint(min_wait, max_wait)
msg = f"The waiting period will be {wait_time} seconds\n"
os.write(2, msg.encode())

time.sleep(wait_time)
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
msg = f"Waited for {wait_time} seconds. Terminating at {current_time}\n"
os.write(1, msg.encode())
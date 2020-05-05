#!/usr/bin/env python3
import multiprocessing
import sys
from queue import Queue

from core import Core


if __name__ == "__main__":
  print('starting sim')
  # who ever has the update semaphore / mutex gets to mess with the queues
  update_semaphore = multiprocessing.Semaphore(1)
  sensor_data = multiprocessing.Queue()
  command = multiprocessing.Queue()
  core = Core(update_semaphore, sensor_data, command)
  try:
    while True:
      pass
      # wait for the update semaphore
      # read the command queue
      # calculate the next time step values appropriately
      # put the appropriate data in the command queue
      # release the update semaphore
  except KeyboardInterrupt as e:
    core.terminate()
    print('terminated all')
    sys.exit(0)


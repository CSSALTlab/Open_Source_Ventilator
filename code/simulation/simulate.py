#!/usr/bin/env python3
import multiprocessing
import sys
import time
import queue


from core import Core


if __name__ == "__main__":
  print('starting sim')
  # who ever has the update semaphore / mutex gets to mess with the queues
  update_lock = multiprocessing.Lock()
  sensor_data = multiprocessing.Queue(1)
  command = multiprocessing.Queue(1)

  core = Core(update_lock, sensor_data, command)

  DELTA_T = 0.001

  STATE = {'time' : 0.0, 'pressure': 0, 'flow': 0}

  try:
    while True:
      # read the command queue
      try:
        new_command = command.get(timeout = DELTA_T)
      except queue.Empty as e:
        new_command = {}
      # calculate the next time step values appropriately
      sensor_data.put(STATE)
      time.sleep(DELTA_T)
      STATE['time'] += DELTA_T
  except KeyboardInterrupt as e:
    core.terminate()
    print('terminated all')
    sys.exit(0)


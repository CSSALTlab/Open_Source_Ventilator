#!/usr/bin/env python3
import threading
import sys
from queue import Queue

from core import Core


if __name__ == "__main__":
  print('starting sim')
  # who ever has the update semaphore / mutex gets to mess with the queues
  update_semaphore = threading.Semaphore(1)
  sensor_data = Queue()
  command = Queue()

  core_thread = threading.Thread(target=Core.spin,
                  args=(update_semaphore, sensor_data, command))
  core_thread.start()
  try:
    while True:
      pass
      # wait for the update semaphore
      # read the command queue
      # calculate the next time step values appropriately
      # put the appropriate data in the command queue
      # release the update semaphore
  except KeyboardInterrupt as e:
    core_thread.stop()
    sys.exit(0)





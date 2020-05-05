#!/usr/bin/env python3
from core import Core
import threading
from queue import Queue



if __name__ == "__main__":
  print('starting sim')
  # who ever has the update semaphore / mutex gets to mess with the queues
  update_seamphore = threading.Semaphore(1)
  sensor_data = Queue()
  command = Queue()

  core_thread = threading.Thread(target=Core.spin(), args=(update_semaphore, sensor_data, command))

#!/usr/bin/env python3
from ctypes import *
import multiprocessing

UPDATE_SEMAPHORE = None
SENSOR_DATA = None
COMMAND = None

class Core(object):
  def __init__(self, update_semaphore, sensor_data, command):
    UPDATE_SEMAPHORE = update_semaphore
    SENSOR_DATA = sensor_data
    COMMAND = command

    core_so = "../objects/libcore.so"
    core_main = CDLL(core_so)

    self.core_process = multiprocessing.Process(target=core_main.main)
    self.core_process.start()


  def terminate(self):
    return self.core_process.terminate()

#TODO: add the hardware interfaces here that will be called from C

# Temaplate for hardware interface get data function
#  wait for time update semaphore
#  get data from the sensore data queue
#  return data

# Temaplate for hardware interface output data function
#  wait for time update semaphore
#  put data in the command queue
#  return


def uart_print(string):
  print('uart: ', string)
  return



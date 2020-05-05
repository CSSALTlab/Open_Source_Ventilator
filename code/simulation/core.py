#!/usr/bin/env python3
from ctypes import *

class Core(object):
  def __init__(self, update_semaphore, sensor_data, command):
    core_so = "../objects/libcore.so"

    core_main = CDLL(core_so)

    print (type(core_main))
    core_main.main()
  #TODO: add the hardware interfaces here that will be called from C

  # Temaplate for hardware interface get data function
  #  wait for time update semaphore
  #  get data from the sensore data queue
  #  return data

  # Temaplate for hardware interface output data function
  #  wait for time update semaphore
  #  put data in the command queue
  #  return


  @classmethod
  def spin(cls, *args):
    core = Core(*args)

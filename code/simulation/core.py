#!/usr/bin/env python3
from ctypes import *
import multiprocessing
import numpy as np


UPDATE_LOCK = None
SENSOR_DATA = None
COMMAND = None

class Core(object):
  def __init__(self, update_lock, sensor_data, command):
    global UPDATE_LOCK
    UPDATE_LOCK = update_lock
    global SENSOR_DATA
    SENSOR_DATA = sensor_data
    global COMMAND
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

'''
Functions to get data to the core
'''
def delay_ms(ms):
  '''
  function to delay the proper amount of time according to sim
    dequeues data from SENSOR_DATA

  returns: None
  '''
  start_time = SENSOR_DATA.get()['time']
  end_time = start_time + ms / 1000

  while True:
    time = SENSOR_DATA.get()['time']
    if time >= end_time:
      break
  return


#TODO
def get_flow():
  '''
  function meant to get instantaneous flow rate (current) from the simualtor
    dequeues data from SENSOR_DATA

  returns: ?
  '''
  pass


#TODO
def get_pressure():
  '''
  function meant to get instantaneous pressure (voltage) from the simualtor
    dequeues data from SENSOR_DATA

  returns: ?
  '''
  pass

#TODO: Buttons



'''
Functions to send data from to the core
'''
def uart_print(string):
  '''
  function meant to simulate printing to the uart

  returns: None
  '''
  print('uart: ', string)
  return


def sound_alarm():
  '''
  function meant to simulate the sounding fo the alarm
  returns: None
   '''
  global COMMAND
  COMMAND.put({'alarm':True})
  print ('Alarm: ON')


def silence_alarm():
  '''
  function to simulate silencing the alarm
  returns: None
  '''
  global COMMAND
  COMMAND.put({'alarm':False})
  print ('Alarm: OFF')


LCD = None
LCD_HEIGHT = 4
LCD_WIDTH = 20

def lcd_print(row, col, string):
  '''
  function to simulate printing to the LCD screen

    row: row to start the string at
    col: col to start the string at
    string: string to print to the simulated

  returns: None
  '''
  global LCD
  if LCD is None:
    LCD = np.zeros((LCD_HEIGHT, LCD_WIDTH), dtype='U1')
    LCD.fill(' ')
  chars = np.array(list(string))[:LCD_WIDTH - col]
  LCD[row, col:] = chars
  s = ''
  for i in range(LCD_HEIGHT):
    for j in range(LCD_WIDTH):
      s += bytes(LCD[i,j], 'utf-8').decode('utf-8')
    s += '\n'
  print ('LCD:')
  print (s)


#TODO: motor / valve outputs

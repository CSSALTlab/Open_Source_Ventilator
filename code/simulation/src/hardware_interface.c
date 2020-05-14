#include "hardware_interface.h"
#include <Python.h>

// Function to call a python funciton from C code.
//   module_name : name of module
//   func_name : name of function in that module
//   pARgs : Py_Tuple
// see uart_print for basic example
PyObject* call_pyfunc(const char* module_name, const char* func_name, PyObject* pArgs)
{
  Py_INCREF(pArgs); // one more person looking at pArgs

  PyObject *pName = NULL;
  PyObject *pModule = NULL;
  PyObject *pFunc = NULL;
  PyObject *pValue = NULL;

  pName = PyUnicode_FromString(module_name);
  pModule = PyImport_Import(pName);
  Py_DECREF(pName); // done with pName

  if (pModule != NULL)
  {
    pFunc = PyObject_GetAttrString(pModule, func_name);
    // pFunc is a new reference
    if (pFunc && PyCallable_Check(pFunc))
    {
      pValue = PyObject_CallObject(pFunc, pArgs);
      // pValue is a new reference
      Py_DECREF(pArgs); // done looking at pArgs
      if (pValue == NULL)
      {
        PyErr_Print();
        fprintf(stderr,"Call to \"%s\" failed\n", func_name);
      }
    }
    else
    {
      // func not found
      if (PyErr_Occurred())
        PyErr_Print();
      fprintf(stderr, "Cannot find function \"%s\"\n", func_name);
    }
    Py_XDECREF(pFunc); // done with pFunc, but not sure if pFunc == NULL
    Py_DECREF(pModule); // done with pModule
  }
  else
  {
    // module not found
    PyErr_Print();
    fprintf(stderr, "Failed to load \"%s\"", module_name);
  }
  return pValue;
}

// template for function in here
//  call approriate python function
//  return value



void initialize_hardware()
{
  Py_Initialize(); // initialization for C Python API
  PyEval_InitThreads(); // initialization for supported threads
  //  (needed because this code is initialized from a separte thread)
  PyGILState_Ensure(); // Acuire the Global Interpreter Lock, we dont need to give back ever
  //  since the other python thread that is running in this proccess just spawns us and thats all
}


// simulates uart output by calling core.uart_print on specified string
void uart_print(char* string)
{
  PyObject* pValue = PyUnicode_FromString(string); // translate from C string to python unicode
  if (pValue == NULL)
  {
      fprintf(stderr, "Cannot convert argument\n");
      return;
  }
  // pValue reference stolen here
  PyObject* pArgs = PyTuple_Pack(1, pValue);

  pValue = call_pyfunc("core", "uart_print", pArgs);

  Py_DECREF(pArgs); // done with args
  Py_DECREF(pValue); // done with returned value
  return;
}


void lcd_print(uint8_t row, uint8_t col, const char* string)
{
  PyObject* pRow = PyLong_FromSize_t(row);
  PyObject* pCol = PyLong_FromSize_t(row);
  PyObject* pString = PyUnicode_FromString(string); // translate from C string to python unicode
  if (pString == NULL || pCol == NULL || pRow == NULL)
  {
      fprintf(stderr, "Cannot convert argument\n");
      return;
  }
  PyObject* pArgs = PyTuple_Pack(3, pRow, pCol, pString);
  PyObject* pRet = call_pyfunc("core", "lcd_print", pArgs);
  Py_DECREF(pArgs); // done with args
  Py_DECREF(pRet); // done with returned value
  return;
}


void sound_alarm()
{
  PyObject* pArgs = PyTuple_New(0);
  PyObject* pRet = call_pyfunc("core", "sound_alarm", pArgs);
  Py_DECREF(pArgs); // done with args
  Py_DECREF(pRet); // done with returned value
  return;
}


void silence_alarm()
{
  PyObject* pArgs = PyTuple_New(0);
  PyObject* pRet = call_pyfunc("core", "silence_alarm", pArgs);
  Py_DECREF(pArgs); // done with args
  Py_DECREF(pRet); // done with returned value
  return;
}


void delay_ms(uint32_t ms)
{
  PyObject* pValue = PyLong_FromSize_t(ms); // translate to python unicode
  if (pValue == NULL)
  {
      fprintf(stderr, "Cannot convert argument\n");
      return;
  }
  // pValue reference stolen here
  PyObject* pArgs = PyTuple_Pack(1, pValue);

  pValue = call_pyfunc("core", "delay_ms", pArgs);

  Py_DECREF(pArgs); // done with args
  Py_DECREF(pValue); // done with returned value
  return;
}

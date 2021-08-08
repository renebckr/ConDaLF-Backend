/**
 * @file python_integration.cpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "python_integration.hpp"

#include <Python.h>
#include <stdlib.h>
#include <common/logging/logging.h>

PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *presult;

// TODO: Don't crash when importing module and we don't have proper env

bool condalf::initialize_python(std::string file)
{
    setenv("PYTHONPATH","./python",1);
    Py_Initialize();

    // Import module
    pName = PyUnicode_FromString(file.c_str());
    pModule = PyImport_Import(pName);

    // Check if any error occured here
    if (PyErr_Occurred() != NULL)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not import Python module. Python Error below:");
        PyErr_Print();
        Py_DECREF(pName);
        return false;
    }

    pDict = PyModule_GetDict(pModule);

    // Check if any error occured here
    if (PyErr_Occurred() != NULL)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not get Dict of Python module. Python Error below:");
        PyErr_Print();
        Py_DECREF(pModule);
        Py_DECREF(pName);
        return false;
    }

    // Get main function
    pFunc = PyDict_GetItemString(pDict, (char*)"process_data");

    // Check if any error occured here
    if (PyErr_Occurred() != NULL)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not get process_data function of Python module.");
        Py_DECREF(pModule);
        Py_DECREF(pName);
        return false;
    }
    return true;
}

void condalf::python_process_data(const std::vector<uint8_t> &data)
{
    if (PyCallable_Check(pFunc))
    {
        pValue = Py_BuildValue("(y#)",&data[0], data.size());

        // Check for error
        if (PyErr_Occurred() != NULL)
        {
            common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not build data value for Python.");
            PyErr_Print();
            return;
        }
        
        presult = PyObject_CallObject(pFunc,pValue);

        // Check for error
        if (PyErr_Occurred() != NULL)
        {
            common::logging::log_error(std::cerr, LINE_INFORMATION, "Error when running python method. See Python Error below:");
            PyErr_Print();
            if (pValue != NULL)
                Py_DECREF(pValue);
        }

        if (pValue != NULL)
            Py_DECREF(pValue);
    }
    else 
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Function is not callable.");
        PyErr_Print();
    }
}

void condalf::uninitialize_python()
{
    // Clean up 
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
}
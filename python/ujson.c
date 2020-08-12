/*
Developed by ESN, an Electronic Arts Inc. studio. 
Copyright (c) 2014, Electronic Arts Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ESN, Electronic Arts Inc. nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS INC. BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Portions of code from MODP_ASCII - Ascii transformations (upper/lower, etc)
http://code.google.com/p/stringencoders/
Copyright (c) 2007  Nick Galbreath -- nickg [at] modp [dot] com. All rights reserved.

Numeric decoder derived from from TCL library
http://www.opensource.apple.com/source/tcl/tcl-14/tcl/license.terms
* Copyright (c) 1988-1993 The Regents of the University of California.
* Copyright (c) 1994 Sun Microsystems, Inc.
*/

#include <Python.h>
#include "version.h"

/* objToJSON */
PyObject* objToJSON(PyObject* self, PyObject *args, PyObject *kwargs);

/* JSONToObj */
PyObject* JSONToObj(PyObject* self, PyObject *args, PyObject *kwargs);

/* objToJSONFile */
PyObject* objToJSONFile(PyObject* self, PyObject *args, PyObject *kwargs);

/* JSONFileToObj */
PyObject* JSONFileToObj(PyObject* self, PyObject *args, PyObject *kwargs);


#define ENCODER_HELP_TEXT "Use ensure_ascii=false to output UTF-8. Pass in double_precision to alter the maximum digit precision of doubles. Set encode_html_chars=True to encode < > & as unicode escape sequences. Set escape_forward_slashes=False to prevent escaping / characters."

static PyMethodDef ujsonMethods[] = {
  {"encode", (PyCFunction) objToJSON, METH_VARARGS | METH_KEYWORDS, "Converts arbitrary object recursively into JSON. " ENCODER_HELP_TEXT},
  {"decode", (PyCFunction) JSONToObj, METH_VARARGS | METH_KEYWORDS, "Converts JSON as string to dict object structure. Use precise_float=True to use high precision float decoder."},
  {"dumps", (PyCFunction) objToJSON, METH_VARARGS | METH_KEYWORDS,  "Converts arbitrary object recursively into JSON. " ENCODER_HELP_TEXT},
  {"loads", (PyCFunction) JSONToObj, METH_VARARGS | METH_KEYWORDS,  "Converts JSON as string to dict object structure. Use precise_float=True to use high precision float decoder."},
  {"dump", (PyCFunction) objToJSONFile, METH_VARARGS | METH_KEYWORDS, "Converts arbitrary object recursively into JSON file. " ENCODER_HELP_TEXT},
  {"load", (PyCFunction) JSONFileToObj, METH_VARARGS | METH_KEYWORDS, "Converts JSON as file to dict object structure. Use precise_float=True to use high precision float decoder."},
  {NULL, NULL, 0, NULL}       /* Sentinel */
};

typedef struct {
  PyObject *type_date;
  PyObject *type_datetime;
  PyObject *type_decimal;
} modulestate;

static int module_traverse(PyObject *m, visitproc visit, void *arg);
static int module_clear(PyObject *m);
static void module_free(void *m);

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "ujson1",
  0,                    /* m_doc */
  sizeof(modulestate),  /* m_size */
  ujsonMethods,         /* m_methods */
  NULL,                 /* m_reload */
  module_traverse,      /* m_traverse */
  module_clear,         /* m_clear */
  module_free           /* m_free */
};

#define modulestate(o) ((modulestate *)PyModule_GetState(o))
#define modulestate_global modulestate(PyState_FindModule(&moduledef))

/* Used in objToJSON.c */
PyObject *PyDate_FromDate(int year, int month, int day)
{
  PyObject *date = modulestate_global->type_date;
  assert(date != NULL);
  return PyObject_CallFunction(date, "(iii)", year, month, day);
}
int object_is_datetime_type(PyObject *obj)
{
  assert(obj != NULL);
  PyObject *datetime = modulestate_global->type_datetime;
  assert(datetime != NULL);
  return PyObject_IsInstance(obj, datetime);
}
int object_is_date_type(PyObject *obj)
{
  assert(obj != NULL);
  PyObject *date = modulestate_global->type_date;
  assert(date != NULL);
  return PyObject_IsInstance(obj, date);
}
int object_is_decimal_type(PyObject *obj)
{
  PyObject *module = PyState_FindModule(&moduledef);
  if (module == NULL) return 0;
  modulestate *state = modulestate(module);
  if (state == NULL) return 0;
  int result = PyObject_IsInstance(obj, state->type_decimal);
  if (result == -1) {
    PyErr_Clear();
    return 0;
  }
  return result;
}

static int module_traverse(PyObject *m, visitproc visit, void *arg)
{
  Py_VISIT(modulestate(m)->type_date);
  Py_VISIT(modulestate(m)->type_datetime);
  Py_VISIT(modulestate(m)->type_decimal);
  return 0;
}

static int module_clear(PyObject *m)
{
  Py_CLEAR(modulestate(m)->type_date);
  Py_CLEAR(modulestate(m)->type_datetime);
  Py_CLEAR(modulestate(m)->type_decimal);
  return 0;
}

static void module_free(void *m)
{
  module_clear((PyObject *)m);
}

PyMODINIT_FUNC PyInit_ujson1(void)
{
  PyObject *module;
  PyObject *version_string;
  PyObject *mod_decimal;
  PyObject *mod_datetime;
  modulestate *state;

  if ((module = PyState_FindModule(&moduledef)) != NULL)
  {
    Py_INCREF(module);
    return module;
  }


  module = PyModule_Create(&moduledef);

  if (module == NULL)
  {
    return NULL;
  }

  mod_decimal = PyImport_ImportModule("decimal");
  if (mod_decimal != NULL)
  {
    state = modulestate(module);
    state->type_decimal = PyObject_GetAttrString(mod_decimal, "Decimal");
    assert(state->type_decimal != NULL);
    Py_DECREF(mod_decimal);
  }
  else
    PyErr_Clear();

  mod_datetime = PyImport_ImportModule("datetime");
  if (mod_datetime != NULL)
  {
    state = modulestate(module);
    state->type_date = PyObject_GetAttrString(mod_datetime, "date");
    assert(state->type_date != NULL);
    state->type_datetime = PyObject_GetAttrString(mod_datetime, "datetime");
    assert(state->type_datetime != NULL);
    Py_DECREF(mod_datetime);
  }
  else
    PyErr_Clear();


  version_string = PyUnicode_FromString (UJSON_VERSION);
  PyModule_AddObject (module, "__version__", version_string);

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}

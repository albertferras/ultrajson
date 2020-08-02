#ifndef CACHEDDICT_H
#define CACHEDDICT_H

#include <Python.h>

typedef struct {
    PyDictObject list;
    PyObject* raw_json;
    size_t offset;
    size_t len;
} CachedDictObject;

CachedDictObject* new_cacheddict();
static PyTypeObject CachedDictType;

PyMODINIT_FUNC PyInit_cacheddict(void);

#endif

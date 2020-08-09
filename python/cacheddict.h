#ifndef CACHEDDICT_H
#define CACHEDDICT_H

#include <Python.h>
#include "cachedobj.h"

typedef struct {
    PyDictObject dict;
    CACHEDOBJ_EXTRA
} CachedDictObject;

extern PyTypeObject CachedDictType;
CachedDictObject* cacheddict_new();
PyObject* init_cacheddict(void);

#endif

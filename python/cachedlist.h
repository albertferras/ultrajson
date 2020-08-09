#ifndef CACHEDLIST_H
#define CACHEDLIST_H

#include <Python.h>
#include "cachedobj.h"

typedef struct {
    PyListObject list;
    CACHEDOBJ_EXTRA
} CachedListObject;

extern PyTypeObject CachedListType;
CachedListObject* cachedlist_new();
PyObject* init_cachedlist(void);

#endif

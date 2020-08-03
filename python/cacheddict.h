#ifndef CACHEDDICT_H
#define CACHEDDICT_H

#include <Python.h>

typedef struct sCachedDictObject CachedDictObject;
struct sCachedDictObject{
    PyDictObject list;
    PyObject* raw_json;
    size_t offset;
    size_t len;
    CachedDictObject* parent_obj;
};

CachedDictObject* new_cacheddict();
PyObject* init_cacheddict(void);

#endif

#ifndef CACHEDDICT_H
#define CACHEDDICT_H

#include <Python.h>

typedef struct sCachedDictObject CachedDictObject;
struct sCachedDictObject{
    PyDictObject dict;
    PyObject* raw_json;
    size_t offset;
    size_t len;
//    CachedDictObject* parent_obj;
    PyObject* parent_obj; // weakref
    PyObject *weakreflist;
};

CachedDictObject* cacheddict_new();
void cacheddict_set_cache();
void cacheddict_set_parent_reference();
PyObject* init_cacheddict(void);

#endif

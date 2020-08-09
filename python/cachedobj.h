#ifndef CACHEDOBJ_H
#define CACHEDOBJ_H

#include <Python.h>

#define CachedObject PyObject  // CachedDictObject or CachedListObject
#define drepr(o) PyObject_Print((PyObject*) o, stdout, Py_PRINT_RAW);

#define CACHEDOBJ_EXTRA                 \
    CacheFields cached;                 \
    PyObject *weakreflist;

typedef struct {
    PyObject* raw_json;
    size_t offset;
    size_t len;
    PyObject* parent_obj; // weakref to obj contained this in json
} CacheFields;

CacheFields* cachefields_new();
void cachefields_dealloc(CacheFields* cache);

CachedObject* cachedobj_new(PyTypeObject* type);
void cachedobj_init(CachedObject* self);
CacheFields* cachedobj_get_cache(CachedObject* self);
void cachedobj_set_cache();
void cachedobj_invalidate(CachedObject *cobj);
void cachedobj_set_parent_reference();
PyObject* cachedobj_get_json(CachedObject *self, PyObject *unused);

int is_cachedobj(CachedObject* self);
void dbg_obj(PyObject* obj, const char* key, int level);
void dbg(CachedObject *cobj, int level);
#endif

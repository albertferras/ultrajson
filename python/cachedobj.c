#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "cacheddict.h"
#include "cachedlist.h"

#define WRAP_METHOD

void
cachefields_dealloc(CacheFields *cache)
{
    Py_XDECREF(cache->raw_json);
    Py_XDECREF(cache->parent_obj);
}


static CachedObject * cachedfields_get_parent(CacheFields* cache) {
    if (cache->parent_obj == NULL)
        return NULL;
    PyObject* ref = PyWeakref_GetObject(cache->parent_obj);
    if (ref == NULL || ref == Py_None) {
        return NULL;
    }
    Py_INCREF(ref);
    return (CachedObject*) ref;
}


void dbg_obj(PyObject* obj, const char* key, int level) {
    if (obj == NULL)
        printf("%d %s = <NULL>", level, key);
    else {
        printf("%d %s (ref=%d) =", level, key, (int) Py_REFCNT(obj));
        drepr(obj);
    }
    printf("\n");
}


void dbg(CachedObject *cobj, int level) {
    PyObject* cstr = cachedobj_get_json(cobj, NULL);
    dbg_obj((PyObject*) cobj, "self", level);
    CacheFields* cache = cachedobj_get_cache(cobj);
    dbg_obj((PyObject*) cache->raw_json, "cache->raw_json", level);
    dbg_obj(cstr, "get_json", level);
    dbg_obj((PyObject*) cache->parent_obj, "cache->parent_obj", level);
    Py_XDECREF(cstr);

    CachedObject* parent_obj = cachedfields_get_parent(cache);
    if (parent_obj != NULL) {
        dbg(parent_obj, level+4);
    }
}


PyObject *
cachedobj_get_json(CachedObject *self, PyObject *unused)
{
    CacheFields* cache = cachedobj_get_cache(self);

    if (cache->raw_json != 0) {
        return PyBytes_FromStringAndSize(
                PyBytes_AsString(cache->raw_json) + cache->offset,
                cache->len
        );
    }
    Py_RETURN_NONE;
}

void cachedobj_invalidate(CachedObject *cobj) {
    // Forget about raw_json because it's not valid for this dict anymore
    CacheFields* cache = cachedobj_get_cache(cobj);
    Py_XDECREF(cache->raw_json);
    cache->raw_json = NULL;

//    printf("invalidating... ");
//    dbg(cobj, 0);

    // Recursively invalidate parent raw_json's
    CachedObject* parent_obj = cachedfields_get_parent(cache);
    if (parent_obj != NULL) {
        cachedobj_invalidate(parent_obj);
        Py_XDECREF(cache->parent_obj);
        cache->parent_obj = NULL;
    }
}

int is_cachedobj(PyObject* self) {
    if (self == NULL)
        return 0;
    CacheFields * cache = cachedobj_get_cache(self);
    if (cache != NULL && cache->raw_json != NULL)
        return 1;
    return 0;
}


void
cachedobj_init(CachedObject* self) {
    CacheFields* cache = cachedobj_get_cache(self);
    if (cache == NULL) {
        printf("Object '%s' is not a cachedobj type\n", Py_TYPE(self)->tp_name);
        return;
    }
    cache->raw_json = NULL;
    cache->offset = 0;
    cache->len = 0;
    cache->parent_obj = NULL;
}


CachedObject*
cachedobj_new(PyTypeObject* type)
{
    CachedObject* cobj = PyObject_CallFunction((PyObject *) type, "");
    if (cobj != NULL)
        cachedobj_init(cobj);
    return cobj;
}

void cachedobj_set_cache(CachedObject* self, PyObject* raw_json, char* start, char* end) {
    CacheFields* cache = cachedobj_get_cache(self);
    cache->raw_json = raw_json;
    Py_INCREF(raw_json);

    char * c_raw_json = PyBytes_AS_STRING(raw_json);
    cache->offset = start - c_raw_json;
    cache->len = end - start;
    cache->parent_obj = NULL;
}

CacheFields* cachedobj_get_cache(CachedObject* self) {
    if (self->ob_type == &CachedDictType) {
        return &(((CachedDictObject *) self)->cached);
    }
    else if (self->ob_type == &CachedListType)
        return &((CachedListObject*) self)->cached;
    return NULL;
}

void cachedobj_set_parent_reference(CachedObject* self, CachedObject* parent) {
    PyObject* ref = PyWeakref_NewRef((PyObject*) parent, NULL);
    if (!ref)
        return;
    if (ref == Py_None)
    {
        Py_DECREF (ref);
        return;
    }
    CacheFields * cache = cachedobj_get_cache(self);
    cache->parent_obj = ref;
}

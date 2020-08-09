#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "cacheddict.h"
#include "cachedobj.h"
#include "py_call_super.h"

#define WRAP_METHOD


static void
CachedDict_dealloc(CachedDictObject *self)
{
    if (self->weakreflist != NULL) {
        PyObject_ClearWeakRefs((PyObject *)self);
    }
    CacheFields* cache = cachedobj_get_cache((CachedObject*) self);
    cachefields_dealloc(cache);
    PyDict_Type.tp_dealloc((PyObject *) self);
}

static int
CachedDict__setitem__(CachedDictObject *self, PyObject *key, PyObject *value) {
    cachedobj_invalidate((CachedObject*) self);
    return PyDict_Type.tp_as_mapping->mp_ass_subscript((PyObject*) self, key, value);
}

static PyObject *
CachedDict_setdefault(CachedDictObject *self, PyObject * args) {
    // todo: only invalidate if key was not in the dict
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "setdefault", args, NULL);
}


static PyObject *
CachedDict_pop(CachedDictObject *self, PyObject * args) {
    // todo: only invalidate if something popped up
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "pop", args, NULL);
}


static PyObject *
CachedDict_popitem(CachedDictObject *self) {
    // todo: only invalidate if something popped up
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "popitem", NULL, NULL);
}


static PyObject *
CachedDict_update(CachedDictObject *self, PyObject* args, PyObject* kwargs) {
    // todo: only invalidate if args/kwargs aren't empty
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "update", args, kwargs);
}

static PyObject *
CachedDict_clear(CachedDictObject *self) {
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "clear", NULL, NULL);
}


static PyMethodDef CachedDict_methods[] = {
        {"__ijson__", (PyCFunction) cachedobj_get_json, METH_NOARGS,
                PyDoc_STR("get cached json string")},

        // todo use METH_FASTCALL and call it like that (dict builtin method uses METH_FASTCALL)
        {"setdefault", (PyCFunction)(void(*)(void))CachedDict_setdefault, METH_VARARGS, ""},
        {"pop", (PyCFunction)(void(*)(void))CachedDict_pop, METH_VARARGS, ""},
        {"popitem", (PyCFunction)(void(*)(void))CachedDict_popitem, METH_NOARGS, ""},
        {"update", (PyCFunction)(void(*)(void))CachedDict_update, METH_VARARGS | METH_KEYWORDS, ""},
        {"clear", (PyCFunction)(void(*)(void))CachedDict_clear, METH_NOARGS, ""},
        {NULL},
};

static PyMappingMethods CachedDict_mappingmethods = {
        (lenfunc)NULL,                       /*mp_length*/
        (binaryfunc)NULL,      /*mp_subscript*/
        (objobjargproc)CachedDict__setitem__,   /*mp_ass_subscript*/
};


static int
CachedDict_init(CachedDictObject *self, PyObject *args, PyObject *kwds)
{
    if (PyDict_Type.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    cachedobj_init((CachedObject*) self);
    return 0;
}


PyTypeObject CachedDictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "CachedDict",
    .tp_doc = "Dict with its source json cached inside, which automatically invalidated when modified",
    .tp_basicsize = sizeof(CachedDictObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) CachedDict_init,
    .tp_dealloc = (destructor) CachedDict_dealloc,
    .tp_weaklistoffset = offsetof(CachedDictObject, weakreflist),
    .tp_methods = CachedDict_methods,
    .tp_as_mapping = &CachedDict_mappingmethods
};


PyObject* init_cacheddict(void)
{
    CachedDictType.tp_base = &PyDict_Type;
    if (PyType_Ready(&CachedDictType) < 0) {
        PyErr_Format(PyExc_TypeError, "Error initialising CachedDictType");
        return NULL;
    }
    return Py_True;
}

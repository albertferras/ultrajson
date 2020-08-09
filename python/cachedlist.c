#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "cachedlist.h"
#include "cachedobj.h"
#include "py_call_super.h"

#define WRAP_METHOD


static PyObject *
CachedList_append(CachedListObject *self, PyObject* args) {
    cachedobj_invalidate((CachedObject*) self);
    return call_super_name((PyObject*) self, "append", args, NULL);
}


static void
CachedList_dealloc(CachedListObject *self)
{
    if (self->weakreflist != NULL) {
        PyObject_ClearWeakRefs((PyObject *)self);
    }
    CacheFields* cache = cachedobj_get_cache((CachedObject*) self);
    cachefields_dealloc(cache);
    PyList_Type.tp_dealloc((PyObject *) self);
}

static PyMethodDef CachedList_methods[] = {
        {"__ijson__", (PyCFunction) cachedobj_get_json, METH_NOARGS,
                PyDoc_STR("get cached json string")},

//        {"append", (PyCFunction)(void(*)(void))CachedList_setdefault, METH_VARARGS, ""},
//        {"insert", (PyCFunction)(void(*)(void))CachedList_pop, METH_VARARGS, ""},
//        {"extend", (PyCFunction)(void(*)(void))CachedList_popitem, METH_NOARGS, ""},
//        {"pop", (PyCFunction)(void(*)(void))CachedList_update, METH_VARARGS | METH_KEYWORDS, ""},
//        {"remove", (PyCFunction)(void(*)(void))CachedList_clear, METH_NOARGS, ""},
//        {"reverse", (PyCFunction)(void(*)(void))CachedList_clear, METH_NOARGS, ""},
//        {"sort", (PyCFunction)(void(*)(void))CachedList_clear, METH_NOARGS, ""},

        {NULL},
};

static PyMappingMethods CachedList_mappingmethods = {
        (lenfunc)NULL,                       /*mp_length*/
        (binaryfunc)NULL,      /*mp_subscript*/
        (objobjargproc) NULL,   /*mp_ass_subscript*/
};


static int
CachedList_init(CachedListObject *self, PyObject *args, PyObject *kwds)
{
    if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
    cachedobj_init((CachedObject*) self);
    return 0;
}


PyTypeObject CachedListType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "CachedList",
    .tp_doc = "List with its source json cached inside, which automatically invalidated when modified",
    .tp_basicsize = sizeof(CachedListObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) CachedList_init,
    .tp_dealloc = (destructor) CachedList_dealloc,
    .tp_weaklistoffset = offsetof(CachedListObject, weakreflist),
    .tp_methods = CachedList_methods,
//    .tp_as_mapping = &CachedList_mappingmethods
};


PyObject* init_cachedlist(void)
{
    CachedListType.tp_base = &PyList_Type;
    if (PyType_Ready(&CachedListType) < 0) {
        PyErr_Format(PyExc_TypeError, "Error initialising CachedListType");
        return NULL;
    }
    return Py_True;
}

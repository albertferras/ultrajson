#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "cacheddict.h"

static PyObject *
CachedDict_get_json(CachedDictObject *self, PyObject *unused)
{
    if (self->raw_json != 0) {
        return PyBytes_FromStringAndSize(
                PyBytes_AsString(self->raw_json) + self->offset,
                self->len);
    }
    // do something here so that serializer knows it has to ignore it?
    return Py_None;
}

static PyMethodDef CachedDict_methods[] = {
    {"__ijson__", (PyCFunction) CachedDict_get_json, METH_NOARGS,
     PyDoc_STR("get cached json string")},
    {NULL},
};

static int
CachedDict_init(CachedDictObject *self, PyObject *args, PyObject *kwds)
{
    if (PyDict_Type.tp_init((PyObject *) self, args, kwds) < 0)
        return -1;
//     todo check kwds
    self->raw_json = 0;
    self->offset = 0;
    self->len = 0;
    return 0;
}

static PyModuleDef cacheddictmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cacheddict",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
};

static PyTypeObject CachedDictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cacheddict.CachedDict",
    .tp_doc = "CachedDict objects",
    .tp_basicsize = sizeof(CachedDictObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) CachedDict_init,
    .tp_methods = CachedDict_methods,
};


CachedDictObject*
new_cacheddict()
{
    CachedDictObject* obj = (CachedDictObject*) PyObject_CallFunction((PyObject *) &CachedDictType, "");
    obj->raw_json = 0;
    obj->offset = 0;
    obj->len = 0;
    return obj;
}

PyMODINIT_FUNC
PyInit_cacheddict(void)
{
    PyObject *m;
    CachedDictType.tp_base = &PyDict_Type;
    if (PyType_Ready(&CachedDictType) < 0)
        return NULL;

    m = PyModule_Create(&cacheddictmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CachedDictType);
    if (PyModule_AddObject(m, "CachedDict", (PyObject *) &CachedDictType) < 0) {
        Py_DECREF(&CachedDictType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

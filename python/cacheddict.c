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
    return Py_None;
}

static cacheddict_touch(CachedDictObject *cobj) {
    // Forget about raw_json because it's not valid for this dict anymore
    cobj->raw_json = NULL;
    Py_XDECREF(cobj->raw_json);

    // Recursively invalidate parent raw_json's
    if (cobj->parent_obj != NULL) {
        cacheddict_touch(cobj->parent_obj);
        Py_XDECREF(cobj->parent_obj);
        cobj->parent_obj = NULL;
    }
}

static dbg(CachedDictObject *cobj, int level) {
    PyObject* cstr = CachedDict_get_json(cobj, Py_None);
    printf("%*sCOBJ=%s\n", level, "", PyBytes_AsString(cstr));
    if (cobj->parent_obj != NULL) {
        dbg(cobj->parent_obj, level+4);
    }
}

static PyObject * CachedDict_dbg(CachedDictObject *cobj, PyObject *unused) {
    dbg(cobj, 0);
    return Py_None;
}


static void
CachedDict_dealloc(CachedDictObject *self)
{
    Py_XDECREF(self->raw_json);
    Py_XDECREF(self->parent_obj);
    PyDict_Type.tp_dealloc((PyObject *) self);
}

static PyObject *
CachedDict__setitem__(PyObject *self, PyObject *args, PyObject *kwds) {
    printf("ARGS: %s\n",
           PyBytes_AsString(
                   PyUnicode_AsEncodedString(PyObject_Repr(args), "utf-8", "~E~")
                   )
           );
//    printf("KARGS: %s\n", PyUnicode_AsEncodedString(PyObject_Repr(kwds)));
//    PyDict_SetItem((PyObject*) self, args);
//    PyDict_Type.tp_as_mapping->mp_ass_subscript((PyObject*) self, v, w);
    return Py_None;
}

static PyMethodDef CachedDict_methods[] = {
        {"__ijson__", (PyCFunction) CachedDict_get_json, METH_NOARGS,
                PyDoc_STR("get cached json string")},
        {"dbg", (PyCFunction) CachedDict_dbg, METH_NOARGS,
         PyDoc_STR("dbg")},
        {"__setitem__", (PyCFunction) CachedDict__setitem__, METH_VARARGS | METH_KEYWORDS,
         PyDoc_STR("set item")},

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
    self->parent_obj = 0;
    return 0;
}

static PyModuleDef cacheddictmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "cacheddict",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
};

PyTypeObject CachedDictType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "cacheddict.CachedDict",
    .tp_doc = "CachedDict objects",
    .tp_basicsize = sizeof(CachedDictObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) CachedDict_init,
    .tp_dealloc = (destructor) CachedDict_dealloc,
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

PyObject* init_cacheddict(void)
{
    CachedDictType.tp_base = &PyDict_Type;
    if (PyType_Ready(&CachedDictType) < 0) {
        PyErr_Format(PyExc_TypeError, "Error initialising CachedDictType");
        return NULL;
    }
    return Py_True;
}

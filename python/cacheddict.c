#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"
#include "cacheddict.h"
#include "py_call_super.h"

#define drepr(o) PyObject_Print((PyObject*) o, stdout, Py_PRINT_RAW);
#define WRAP_METHOD

static PyObject *
CachedDict_get_json(CachedDictObject *self, PyObject *unused)
{
    if (self->raw_json != 0) {
        return PyBytes_FromStringAndSize(
                PyBytes_AsString(self->raw_json) + self->offset,
                self->len);
    }
    Py_RETURN_NONE;
}



static CachedDictObject * cachedobj_get_parent(CachedDictObject *cobj) {
    if (cobj->parent_obj == NULL)
        return NULL;
    PyObject* ref = PyWeakref_GetObject(cobj->parent_obj);
    if (ref == NULL || ref == Py_None) {
        return NULL;
    }
    Py_INCREF(ref);
    return (CachedDictObject*) ref;
}


static void dbg_obj(PyObject* obj, const char* key, int level) {
    if (obj == NULL)
        printf("%d %s = <NULL>", level, key);
    else {
        printf("%d %s (ref=%d) =", level, key, (int) Py_REFCNT(obj));
        drepr(obj);
    }
    printf("\n");
}


static dbg(CachedDictObject *cobj, int level) {
    PyObject* cstr = CachedDict_get_json(cobj, NULL);
    dbg_obj((PyObject*) cobj, "self", level);
    dbg_obj((PyObject*) cobj->raw_json, "self->raw_json", level);
    dbg_obj(cstr, "get_json", level);
    dbg_obj((PyObject*) cobj->parent_obj, "self->parent_obj", level);
    Py_XDECREF(cstr);

    CachedDictObject* parent_obj = cachedobj_get_parent(cobj);
    if (parent_obj != NULL) {
    dbg(parent_obj, level+4);
    }
}

static PyObject * CachedDict_dbg(CachedDictObject *cobj, PyObject *unused) {
    dbg(cobj, 0);
    Py_RETURN_NONE;
}

static void cachedobj_invalidate(CachedDictObject *cobj) {
    // Forget about raw_json because it's not valid for this dict anymore
    Py_XDECREF(cobj->raw_json);
    cobj->raw_json = NULL;
    printf("invalidating... ");
    dbg(cobj, 0);

    // Recursively invalidate parent raw_json's
    CachedDictObject* parent_obj = cachedobj_get_parent(cobj);
    if (parent_obj != NULL) {
        cachedobj_invalidate(parent_obj);
        Py_XDECREF(cobj->parent_obj);
        cobj->parent_obj = NULL;
    }
}

static void
CachedDict_dealloc(CachedDictObject *self)
{
    if (self->weakreflist != NULL) {
        PyObject_ClearWeakRefs((PyObject *)self);
    }

    Py_XDECREF(self->raw_json);
    Py_XDECREF(self->parent_obj);
    PyDict_Type.tp_dealloc((PyObject *) self);
}

static int
CachedDict__setitem__(CachedDictObject *self, PyObject *key, PyObject *value) {
    cachedobj_invalidate(self);
    return PyDict_Type.tp_as_mapping->mp_ass_subscript((PyObject*) self, key, value);
}

static PyObject *
CachedDict_setdefault(CachedDictObject *self, PyObject * args) {
    // todo: only invalidate if key was not in the dict
    cachedobj_invalidate(self);
    return call_super_name((PyObject*) self, "setdefault", args, NULL);
}


static PyObject *
CachedDict_pop(CachedDictObject *self, PyObject * args) {
    // todo: only invalidate if something popped up
    cachedobj_invalidate(self);
    return call_super_name((PyObject*) self, "pop", args, NULL);
}


static PyObject *
CachedDict_popitem(CachedDictObject *self) {
    // todo: only invalidate if something popped up
    cachedobj_invalidate(self);
    return call_super_name((PyObject*) self, "popitem", NULL, NULL);
}


static PyObject *
CachedDict_update(CachedDictObject *self, PyObject* args, PyObject* kwargs) {
    // todo: only invalidate if args/kwargs aren't empty
    cachedobj_invalidate(self);
    return call_super_name((PyObject*) self, "update", args, kwargs);
}

static PyObject *
CachedDict_clear(CachedDictObject *self) {
    cachedobj_invalidate(self);
    return call_super_name((PyObject*) self, "clear", NULL, NULL);
}


static PyMethodDef CachedDict_methods[] = {
        {"__ijson__", (PyCFunction) CachedDict_get_json, METH_NOARGS,
                PyDoc_STR("get cached json string")},
        {"dbg", (PyCFunction) CachedDict_dbg, METH_NOARGS,
         PyDoc_STR("dbg")},

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
    .tp_weaklistoffset = offsetof(CachedDictObject, weakreflist),
    .tp_methods = CachedDict_methods,
    .tp_as_mapping = &CachedDict_mappingmethods
};


CachedDictObject*
cacheddict_new()
{
    CachedDictObject* obj = (CachedDictObject*) PyObject_CallFunction((PyObject *) &CachedDictType, "");
    obj->raw_json = 0;
    obj->offset = 0;
    obj->len = 0;
    return obj;
}

void cacheddict_set_cache(CachedDictObject* obj, PyObject* raw_json_str_obj, char* start, char* end) {
    CachedDictObject* cobj = (CachedDictObject*) obj;
    cobj->raw_json = raw_json_str_obj;
    Py_INCREF(raw_json_str_obj);

    char * c_raw_json = PyBytes_AS_STRING(raw_json_str_obj);
    cobj->offset = start - c_raw_json;
    cobj->len = end - start;
    cobj->parent_obj = NULL;
}

void cacheddict_set_parent_reference(CachedDictObject* obj, CachedDictObject* parent) {
    CachedDictObject * cobj = ((CachedDictObject*) obj);
    PyObject* ref = PyWeakref_NewRef((PyObject*) parent, NULL);
    if (!ref)
        return;
    if (ref == Py_None)
    {
        Py_DECREF (ref);
        return;
    }
    cobj->parent_obj = ref;
//    Py_INCREF(cobj->parent_obj);
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

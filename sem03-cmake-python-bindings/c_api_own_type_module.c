// %%cpp c_api_own_type_module.c
// %run clang -Wall c_api_own_type_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_own_type_module.so
#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
    PyObject_HEAD;
    double x, y;
} PyPoint;

PyTypeObject py_point_type = {    
    PyVarObject_HEAD_INIT(NULL, 0)
};   

PyObject* PyPoint_new(PyTypeObject* type, PyObject* args, PyObject* kwargs) {
    return (PyObject*)type->tp_alloc(type, 0);
}

void PyPoint_dealloc(PyPoint* self) {
    Py_TYPE(self)->tp_free(self);
}

int PyPoint_init(PyPoint* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"x", "y", NULL};
    self->x = self->y = 0;
    return PyArg_ParseTupleAndKeywords(args, kwargs, "|dd", (char**)kwlist, &self->x, &self->y) != 0;
}


PyPoint* PyPoint_setfrom(PyPoint* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"data", NULL};
    char* val_s = ""; Py_ssize_t val_s_len = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "z#", (char**)kwlist, &val_s, &val_s_len)) {
        return NULL;
    }
    sscanf(val_s, "{%lf, %lf}", &self->x, &self->y);
    Py_INCREF(self);
    return self;
}

PyPoint* PyPoint_add(PyPoint* self, PyPoint* arg) {
    if (!PyObject_IsInstance((PyObject*)arg, (PyObject*)Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "not Point type");
        return NULL; 
    }
    PyPoint* result = (PyPoint*)PyPoint_new(&py_point_type, NULL, NULL);
    result->x = self->x + arg->x;
    result->y = self->y + arg->y;
    Py_INCREF(result);
    return result;
}

PyObject* PyPoint_repr(PyPoint* self)
{
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "{%.1lf, %.1lf}", self->x, self->y);
    return PyUnicode_FromString(buffer);
}


void PyPoint_prepare_type() {
    PyTypeObject* o = &py_point_type;
    o->tp_name = "Point";
    o->tp_basicsize = sizeof(PyPoint);
    o->tp_dealloc = (destructor)PyPoint_dealloc;
    o->tp_repr = (reprfunc)PyPoint_repr;
    static PyNumberMethods number_methods = {
        .nb_add = (binaryfunc)PyPoint_add,
    };
    o->tp_as_number = &number_methods;
    o->tp_str = (reprfunc)PyPoint_repr;
    o->tp_flags = Py_TPFLAGS_DEFAULT;
    o->tp_doc = "Just a 2d point\n";
    static PyMethodDef methods[] = {
        {"setfrom", (PyCFunction)PyPoint_setfrom, METH_VARARGS|METH_KEYWORDS, NULL},
        {NULL, NULL, 0, NULL}
    };
    o->tp_methods = methods;
    o->tp_init = (initproc)PyPoint_init;
    o->tp_new = PyPoint_new;
    if (PyType_Ready(o) < 0) {
        Py_FatalError("Can't initialize 'Point'");
    }
}

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_own_type_module(void) {
    static struct PyModuleDef mod_obj = {
        PyModuleDef_HEAD_INIT, "c_api_own_type_module", "Test class module", -1, NULL
    };
    PyObject* mod = PyModule_Create(&mod_obj);
    PyPoint_prepare_type();
    Py_INCREF(&py_point_type);
    PyModule_AddObject(mod, "Point", (PyObject*)&py_point_type);
    return mod;
}


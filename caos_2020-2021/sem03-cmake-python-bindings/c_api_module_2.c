// %%cpp c_api_module_2.c
// %run clang -Wall c_api_module_2.c $(python3-config --includes --ldflags) -shared -fPIC -o c_api_module_2.so
#include <Python.h>

static PyObject* print_dict(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"d", NULL};
    PyObject* d;
    // O - any object and pass just &d, O! - object of chosen type and pass &PyDict_Type, &d
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!", (char**)kwlist, &PyDict_Type, &d)) {
        return NULL;
    }
    Py_ssize_t ppos = 0;
    PyObject* pkey; PyObject* pvalue;
    while (PyDict_Next(d, &ppos, &pkey, &pvalue)) {
        const char* key = PyUnicode_AsUTF8(pkey);
        if (!key) return NULL;
        char value_storage[20];
        const char* value = value_storage; 
        if (PyLong_Check(pvalue)) {
            sprintf(value_storage, "%lld", PyLong_AsLongLong(pvalue));
        } else {
            value = PyUnicode_AsUTF8(pvalue);
            if (!value) return NULL;
        }
        
        printf("%s -> %s\n", key, value);
    }
    printf("\n");
    fflush(stdout);

    Py_RETURN_NONE; // Инкрементит счетчик ссылок None и возвращает его
}

// Список функций модуля
static PyMethodDef methods[] = {
    {"print_dict", (PyCFunction)print_dict, METH_VARARGS | METH_KEYWORDS, "print_dict"},
    {NULL, NULL, 0, NULL}
};

// Описание модуля
static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "c_api_module_2", "Test module", -1, methods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_module_2(void) {
    PyObject* mod = PyModule_Create(&module);
    return mod;
}


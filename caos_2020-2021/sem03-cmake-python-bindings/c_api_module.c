// %%cpp c_api_module.c
// %// Собираем модуль - динамическую библиотеку. Включаем нужные пути для инклюдов и динамические библиотеки
// %run gcc -Wall c_api_module.c $(python3-config --includes --ldflags) -shared -fPIC -fsanitize=address -o c_api_module.so
#define PY_SSIZE_T_CLEAN
#include <Python.h>

// Парсинг позиционных аргументов в лоб
static PyObject* func_1_(PyObject* self, PyObject* args) {
    if (PyTuple_Size(args) != 2) {
        PyErr_SetString(PyExc_TypeError, "func_ret_str args error"); // выставляем ошибку
        return NULL; // возвращаем NULL - признак ошибки
    }
    long int val_i; char *val_s;
    // l - long int, s - char*
    if (!PyArg_ParseTuple(args, "ls", &val_i, &val_s)) {
        return NULL;
    }
    printf("func1: int - %ld, string - %s\n", val_i, val_s);
    return Py_BuildValue("ls", val_i, val_s);
}

// Умный парсинг args и kwargs
static PyObject* func_2(PyObject* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"val_i", "val_s", NULL};
    long int val_i = 0; char* val_s = ""; Py_ssize_t val_s_len = 0;
    // до | обязательные аргументы, l - long int, z# - char* + size_t
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l|z#", (char**)kwlist, &val_i, &val_s, &val_s_len)) {
        return NULL; // ошибка уже выставлена функцией PyArg_ParseTupleAndKeywords
    }
    printf("func2: int - %ld, string - %s, string_len = %d\n", val_i, val_s, (int)val_s_len);
    return Py_BuildValue("ls", val_i, val_s);
}

// Список функций модуля
static PyMethodDef methods[] = {
    {"func_1", func_1_, METH_VARARGS, "help func_1"},
    // METH_KEYWORDS - принимает еще и именованные аргументы
    {"func_2", (PyCFunction)func_2, METH_VARARGS | METH_KEYWORDS, "help func_2"},
    {NULL, NULL, 0, NULL}
};

// Описание модуля
static struct PyModuleDef mod = {
    PyModuleDef_HEAD_INIT, "c_api_module", "Test module", -1, methods
};

// Инициализация модуля
PyMODINIT_FUNC PyInit_c_api_module(void) {
    return PyModule_Create(&mod);
}


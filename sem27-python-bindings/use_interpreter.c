// %%cpp use_interpreter.c
// %run clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -o use_interpreter.exe
// %run ./use_interpreter.exe
#include <Python.h>

int main() {
    Py_Initialize();
    PyObject* locals = PyDict_New();
    PyObject* globals = PyDict_Copy(PyEval_GetBuiltins()); // Нам же нужно, чтобы функция print сразу была определена?
    
    const char* exec_prefix = "$ ";
    const char* cmds[] = {
        "40 + 2",
        "print(1)",
        "$ a = 40 + 2",
        "$ b = 5 + 5",
        "print(a * b)",
        "a + b",
        ("$ " 
            "for i in range(3):"                    "\n"
            "    print('i = %d' % i, end=', ')"     "\n"
            "print()"                               "\n"
        ),
        "&",
        NULL
    };
    
    for (const char** line = cmds; *line; ++line) {
        PyObject* result = (strncmp(line[0], exec_prefix, strlen(exec_prefix)) == 0) ?    // eval or exec string?
            PyRun_String(line[0] + strlen(exec_prefix), Py_file_input, globals, locals) : // exec
            PyRun_String(line[0], Py_eval_input, globals, locals);                        // eval
        if (result) {
            PyObject_Print(result, stdout, 0); printf("\n"); // печать python-объекта
            Py_DECREF(result);
        } else {
            // Не забываем, что python-функции возвращают None если нормально завершаются без return и исключений
            // При этом None это специальный синглтон. То есть != NULL. 
            // А вот если функция вернула NULL, то это значит, что кинуто исключение
            PyErr_PrintEx(0); // печать исключения
            PyErr_Clear();
        }
        
    }
    
    Py_DECREF(locals);
    Py_DECREF(globals);
    Py_Finalize();
}


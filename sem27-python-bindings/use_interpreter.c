// %%cpp use_interpreter.c
// %run clang -Wall use_interpreter.c $(python3-config --includes --ldflags) -o use_interpreter.exe
// %run ./use_interpreter.exe
#include <Python.h>


int main() {
    Py_Initialize();
    PyObject* locals = PyDict_New();
    PyObject* globals = PyDict_Copy(PyEval_GetBuiltins()); 
    
    const char* exec_prefix = "$ ";
    const char* cmds[] = {
        "40 + 2",
        "print(1)",
        "$ a = 40 + 2",
        "$ b = 5 + 5",
        "print(a * b)",
        "a + b",
        "&",
        NULL
    };
    
    PyRun_SimpleString("print('Hello!')");
    for (const char** line = cmds; *line; ++line) {
        PyObject* result = (strncmp(line[0], exec_prefix, strlen(exec_prefix)) == 0) ?
            PyRun_String(line[0] + strlen(exec_prefix), Py_file_input, globals, locals) :
            PyRun_String(line[0], Py_eval_input, globals, locals);
        if (result) {
            PyObject_Print(result, stdout, 0); printf("\n");
        } else {
            PyErr_PrintEx(0);
            PyErr_Clear();
        }
        
    }
    Py_Finalize();
}


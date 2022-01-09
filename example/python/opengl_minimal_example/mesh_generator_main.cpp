#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string>
#include <iostream>

int length_of_filename(std::string filename)
{
    int length = static_cast<int>(filename.size());
    return length;
}

extern "C"
{
    static PyObject * mesh_generator_read(PyObject *self, PyObject *args)
    {
        int verbose;
        const char *filename;
        if (!PyArg_ParseTuple(args, "is", &verbose, &filename))
            return NULL;

        int length = length_of_filename(filename);
        if (verbose)
            std::cout << "length(\"" << filename << "\"): " << length << std::endl;
        //return PyLong_FromLong(length);
        return Py_BuildValue("i", length);
    }

    static PyMethodDef mesh_generator_methods[] = {
        
        {"read",  (PyCFunction)mesh_generator_read, METH_VARARGS, "Execute read."},
        {NULL, NULL, 0, NULL} // Sentinel
    };

    char mesh_generator_docs[] = "generate meshes.";

    static struct PyModuleDef mesh_generator_module = {
        PyModuleDef_HEAD_INIT,
        "mesh_generator",
        mesh_generator_docs,
        -1,
        mesh_generator_methods
    };

    PyMODINIT_FUNC
    PyInit_mesh_generator(void)
    {
        return PyModule_Create(&mesh_generator_module);
    }
}

int main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    if (PyImport_AppendInittab("spam", PyInit_mesh_generator) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    Py_SetProgramName(program);
    Py_Initialize();

    PyObject *pmodule = PyImport_ImportModule("mesh_generator");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'testcpp'\n");
    }

    PyMem_RawFree(program);
    return 0;
}
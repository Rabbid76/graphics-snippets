#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <vector>
#include <numeric>
#include <string>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535f
#endif

void generate_spiral_mesh(
    float rounds, float height, float thickness, float radius,
    int slices, int step,
    std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    for (int i = -slices; i <= rounds * 360 + step; i += step)
    {
        for (int j = 0; j < slices; j++)
        {
            float t = (float)i / 360 + (float)j / slices * step / 360;
            t = std::max(0.0f, std::min(rounds, t));
            float a1 = t * (float)M_PI * 2.0f;
            float a2 = (float)j / slices * (float)M_PI * 2.0f;
            float d = radius + thickness * cos(a2);
            vertices.push_back(d * std::cos(a1));
            vertices.push_back(d * std::sin(a1));
            vertices.push_back(thickness * sin(a2) + height * t / rounds - height / 2);
        }
    }
    for (unsigned int i = 0; i < (unsigned int)vertices.size() / 3 - slices; ++i)
    {
        indices.push_back(i);
        indices.push_back(i + slices);
    }
}

extern "C"
{
    static PyObject * mesh_generate_spiral_mesh(PyObject *self, PyObject *args)
    {
        float rounds, height, thickness, radius;
        int slices, step;
        if (!PyArg_ParseTuple(args, "ffffii", &rounds, &height, &thickness, &radius, &slices, &step))
            return nullptr;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        generate_spiral_mesh(rounds, height, thickness, radius, slices, step, vertices, indices);

        return Py_BuildValue(
            "iy#iy#", 
            vertices.size(), vertices.data(), vertices.size() * sizeof(*vertices.data()),
            indices.size(), indices.data(), indices.size() * sizeof(*indices.data()));
    }

    static PyMethodDef mesh_generator_methods[] = {
        
        {"generate_spiral",  (PyCFunction)mesh_generate_spiral_mesh, METH_VARARGS, "generate spiral"},
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
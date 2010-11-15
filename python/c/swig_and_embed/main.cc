#include <Python.h>

#include "../auto_pyobj.h"
#include "nativetype.h"

// These functions are defined in swig autogenerated code.
PyObject* wrapRootInstance(const Root& root);
extern "C" {
void init_nativetype();
}

bool append_sys_path(const char* path) {
  PyObject* path_list = PySys_GetObject(const_cast<char*>("path"));
  if (path_list == NULL || !PyList_Check(path_list)) {
    PyErr_SetString(PyExc_ImportError,
                    "sys.path must be a list of directory names");
    return false;
  }
  PyObject* path_str = PyString_FromString(path);
  if (path_str == NULL) {
    return false;
  }
  PyList_Append(path_list, path_str);
  return true;
}

int main_routine() {
  // Appends current directory to sys.path to import nativetype.py
  if (!append_sys_path("")) {
    PyErr_Print();
    return 1;
  }
  // Loads native swig codes
  init_nativetype();
  // Loads Python swig codes
  auto_pyobj nativetype_name(PyString_FromString("nativetype"));
  PyObject* nativetype_module = PyImport_Import(nativetype_name.get());
  if (!nativetype_module) {
    PyErr_Print();
    return 1;
  }
  // Creates native objects.
  Leaf left(3);
  Leaf right(4);
  Root root(left, right);
  // Wraps native objects
  PyObject* pyroot = wrapRootInstance(root);
  // Stores the object in __main__ module, in which 
  PyObject* main_module = PyImport_AddModule("__main__");
  PyObject* module_dict = PyModule_GetDict(main_module);
  PyDict_SetItemString(module_dict, "root", pyroot);
  // PyRun_SimpleString (in Python/pythonrun.c) executes given codes
  // in __main__, implicitly.
  PyRun_SimpleString("print 'root.left.value == ', "
                     "root.get_left().get_value();");
  PyRun_SimpleString("print 'root.right.value == ', "
                     "root.get_right().get_value();");
  // In Python, root.get_left() != root.get_left() because Swig creates
  // different Python instances for a same native instance.
  PyRun_SimpleString("print 'root.get_left() == root.get_left():',"
                     "root.get_left() == root.get_left();");
  return 0;
}

int main(int argc, char *argv[]) {
  Py_Initialize();
  int rc = main_routine();
  Py_Finalize();
  return rc;
}
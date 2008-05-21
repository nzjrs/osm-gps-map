#include <pygobject.h>

void pyosmgpsmap_register_classes(PyObject *d);
extern PyMethodDef pyosmgpsmap_functions[];

DL_EXPORT(void)
initosmgpsmap(void)
{
	PyObject *m, *d;

	init_pygobject();

	m = Py_InitModule("osmgpsmap", pyosmgpsmap_functions);
	d = PyModule_GetDict(m);

	pyosmgpsmap_register_classes(d);

	/* Add this if we ever add an enum or something to osmgpsmap. */
#if 0
	pyosmgpsmap_add_constants(m, "OSM_GPS_MAP_");
#endif

	if (PyErr_Occurred()) {
		Py_FatalError("can't initialize module osmgpsmap");
	}
}

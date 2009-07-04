/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osmgpsmapmodule.
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * osm-gps-map.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * osm-gps-map.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pygobject.h>
#include <osm-gps-map.h>

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
    pyosmgpsmap_add_constants(m, "SOURCE_");

	/* Add this if we ever add an enum or something to osmgpsmap. */
#if 0
	pyosmgpsmap_add_constants(m, "OSM_GPS_MAP_");
#endif

#if 0
	/* Manually add all the Map repository strings */
    PyModule_AddObject(m, "MAP_SOURCE_OPENSTREETMAP",
		       PyString_FromString(MAP_SOURCE_OPENSTREETMAP));
    PyModule_AddObject(m, "MAP_SOURCE_OPENSTREETMAP_RENDERER",
		       PyString_FromString(MAP_SOURCE_OPENSTREETMAP_RENDERER));
    PyModule_AddObject(m, "MAP_SOURCE_OPENAERIALMAP",
		       PyString_FromString(MAP_SOURCE_OPENAERIALMAP));
    PyModule_AddObject(m, "MAP_SOURCE_GOOGLE_HYBRID",
		       PyString_FromString(MAP_SOURCE_GOOGLE_HYBRID));
    PyModule_AddObject(m, "MAP_SOURCE_GOOGLE_SATTELITE",
		       PyString_FromString(MAP_SOURCE_GOOGLE_SATTELITE));
    PyModule_AddObject(m, "MAP_SOURCE_GOOGLE_SATTELITE_QUAD",
		       PyString_FromString(MAP_SOURCE_GOOGLE_SATTELITE_QUAD));
    PyModule_AddObject(m, "MAP_SOURCE_MAPS_FOR_FREE",
		       PyString_FromString(MAP_SOURCE_MAPS_FOR_FREE));
    PyModule_AddObject(m, "MAP_SOURCE_VIRTUAL_EARTH_SATTELITE",
		       PyString_FromString(MAP_SOURCE_VIRTUAL_EARTH_SATTELITE));
#endif

	if (PyErr_Occurred()) {
		Py_FatalError("can't initialize module osmgpsmap");
	}
}

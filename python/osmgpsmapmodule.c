/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osmgpsmapmodule.
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <pygobject.h>
#include <osm-gps-map.h>
#include <osm-gps-map-osd.h>
#include <osm-gps-map-layer.h>

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
    pyosmgpsmap_add_constants(m, "OSM_GPS_MAP_");

	PyModule_AddObject(m, "INVALID",
		PyFloat_FromDouble(OSM_GPS_MAP_INVALID));
	PyModule_AddObject(m, "CACHE_DISABLED",
		PyString_FromString(OSM_GPS_MAP_CACHE_DISABLED));
	PyModule_AddObject(m, "CACHE_AUTO",
		PyString_FromString(OSM_GPS_MAP_CACHE_AUTO));
	PyModule_AddObject(m, "CACHE_FRIENDLY",
		PyString_FromString(OSM_GPS_MAP_CACHE_FRIENDLY));

	if (PyErr_Occurred()) {
		Py_FatalError("can't initialize module osmgpsmap");
	}
}

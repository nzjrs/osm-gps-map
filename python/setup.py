#!/usr/bin/env python

import os
from distutils.core import setup, Extension

def pkg_config_parse(pkg, opt):
    conf = os.popen('pkg-config %s %s' % (opt,pkg)).read()
    opt = opt[-2:]
    return [x.lstrip(opt) for x in conf.split()]

def get_include(pkg):
    return pkg_config_parse(pkg,'--cflags-only-I')

def get_lib_dirs(pkg):
    return pkg_config_parse(pkg,'--libs-only-L')

def get_libs(pkg):
    return pkg_config_parse(pkg,'--libs-only-l')

VERSION = "0.7.3"

_osmgpsmap = Extension(name = 'osmgpsmap',
            sources= ['osmgpsmapmodule.c','osmgpsmap.c'],
            include_dirs = get_include('osmgpsmap pygobject-2.0'),
            library_dirs = get_lib_dirs('osmgpsmap pygobject-2.0'),
            libraries = get_libs('osmgpsmap pygobject-2.0'),
            define_macros = [('VERSION', '"""%s"""' % VERSION)],
            )

setup( name = "python-osmgpsmap",
    version = VERSION,
    description = "python interface for osmgpsmap",
    ext_modules = [_osmgpsmap],
    )


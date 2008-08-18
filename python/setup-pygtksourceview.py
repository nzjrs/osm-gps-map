#!/usr/bin/env python
#
# setup.py - distutils configuration for pygtksourceview
#
"""Python Bindings for GtkSourceView."""

from distutils.command.build import build
from distutils.core import setup
import glob
import os
import sys

import dsextras

if not dsextras.have_pkgconfig():
    print "Error, could not find pkg-config"
    raise SystemExit

# we need this hack to make dsextras Template work since it uses
# the codegen module:
# we locate the module path and then reload dsextras.
codegendir = dsextras.getoutput('pkg-config --variable codegendir pygtk-2.0')
codegendir = os.path.dirname(codegendir)
sys.path.append(codegendir)
try:
    import codegen.createdefs
except ImportError:
    raise SystemExit, \
'Could not find code generator in %s, do you have installed pygtk correctly?'
reload(dsextras)

from dsextras import get_m4_define, getoutput, have_pkgconfig, \
     pkgc_version_check, \
     GLOBAL_INC, GLOBAL_MACROS, InstallLib, InstallData, BuildExt, \
     PkgConfigExtension, Template, TemplateExtension

if '--yes-i-know-its-not-supported' in sys.argv:
    sys.argv.remove('--yes-i-know-its-not-supported')
else:
    print '*'*70
    print 'Building PyGtkSourceView using distutils is NOT SUPPORTED.'
    print "It's mainly included to be able to easily build win32 installers"
    print "You may continue, but only if you agree to not ask any questions"
    print "To build PyGObject in a supported way, read the INSTALL file"
    print
    print "Build fixes are of course welcome and should be filed in bugzilla"
    print '*'*70
    input = raw_input('Not supported, ok [y/N]? ')
    if not input.startswith('y'):
        raise SystemExit

if sys.version_info[:3] < (2, 3, 5):
    raise SystemExit, \
          "Python 2.3.5 or higher is required, %d.%d.%d found" % sys.version_info[:3]

MAJOR_VERSION = int(get_m4_define('pygtksourceview_major_version'))
MINOR_VERSION = int(get_m4_define('pygtksourceview_minor_version'))
MICRO_VERSION = int(get_m4_define('pygtksourceview_micro_version'))

VERSION = "%d.%d.%d" % (MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION)

GTKSOURCEVIEW_REQUIRED = get_m4_define('gtksourceview_required_version')
PYGOBJECT_REQUIRED  = get_m4_define('pygobject_required_version')
PYGTK_REQUIRED  = get_m4_define('pygtk_required_version')

GLOBAL_INC += ['gtksourceview']
GLOBAL_MACROS += [('PYGTKSOURCEVIEW_MAJOR_VERSION', MAJOR_VERSION),
                  ('PYGTKSOURCEVIEW_MINOR_VERSION', MINOR_VERSION),
                  ('PYGTKSOURCEVIEW_MICRO_VERSION', MICRO_VERSION)]

if sys.platform == 'win32':
    GLOBAL_MACROS.append(('VERSION', '"""%s"""' % VERSION))
    GLOBAL_MACROS.append(('PLATFORM_WIN32',1))
    GLOBAL_MACROS.append(('HAVE_BIND_TEXTDOMAIN_CODESET',1))
else:
    GLOBAL_MACROS.append(('VERSION', '"%s"' % VERSION))

defsdir = getoutput('pkg-config --variable defsdir pygtk-2.0')
print "defs dir " + defsdir
GTKDEFS = [os.path.join(defsdir, 'pango-types.defs'),
           os.path.join(defsdir, 'gdk-types.defs'),
           os.path.join(defsdir, 'gtk-types.defs')]

PYGTK_SUFFIX = '2.0'
DEFS_DIR = os.path.join('share', 'pygtk', PYGTK_SUFFIX, 'defs')
HTML_DIR = os.path.join('share', 'gtk-doc', 'html', 'pygtksourceview2')


class PyGtkSourceViewInstallData(InstallData):
    def run(self):
        self.add_template_option('VERSION', VERSION)
        self.prepare()

        # Install templates
        self.install_templates()

        InstallData.run(self)

    def install_templates(self):
        self.install_template('pygtksourceview-2.0.pc.in',
                              os.path.join(self.install_dir,
                                           'lib', 'pkgconfig'))

gtksourceview2 = TemplateExtension(name='gtksourceview2',
				   pkc_name='gtksourceview-2.0',
                                   pkc_version=GTKSOURCEVIEW_REQUIRED,
                                   sources=['gtksourceview2module.c', 'gtksourceview2.c'],
                                   register=GTKDEFS,
                                   override='gtksourceview2.override',
                                   defs='gtksourceview2.defs',
                                   py_ssize_t_clean=True)

data_files = []
ext_modules = []

if gtksourceview2.can_build():
    ext_modules.append(gtksourceview2)
    data_files.append((DEFS_DIR, ('gtksourceview2.defs',)))
    data_files.append((HTML_DIR, glob.glob('docs/html/*.html')))
else:
    raise SystemExit

doclines = __doc__.split("\n")

options = {"bdist_wininst": {"install_script": "pygtksourceview_postinstall.py"}}

setup(name="pygtksourceview",
      version=VERSION,
      license='LGPL',
      platforms=['yes'],
      description = doclines[0],
      long_description = '\n'.join(doclines[2:]),
      ext_modules = ext_modules,
      data_files = data_files,
      scripts = ['pygtksourceview_postinstall.py'],
      options = options,
      cmdclass = {
	  'install_data': PyGtkSourceViewInstallData,
	  'build_ext': BuildExt,
      }
)

#!/usr/bin/env python

from distutils.core import setup, Extension
import sys
import os.path
import shutil
import sys
import numpy

shutil.copyfile(os.path.join('python','MANIFEST.python'),'MANIFEST')

extra_link_args = []
if sys.platform != 'darwin':
    extra_link_args = ['-s']


extension = Extension("Pycluster.cluster",
                      ["src/cluster.c", "python/clustermodule.c"],
                      include_dirs=['src', numpy.get_include()],
                      extra_link_args=extra_link_args
                      )


setup(name="Pycluster",
      version="1.43",
      description="The C Clustering Library",
      author="Michiel de Hoon",
      author_email="mdehoon 'AT' gsc.riken.jp",
      url="http://bonsai.ims.u-tokyo.ac.jp/~mdehoon/software/software.html",
      license="Python License",
      package_dir = {'Pycluster':'python'},
      packages = ['Pycluster'],
      ext_modules=[extension]
      )

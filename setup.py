#!/usr/bin/env python

from distutils.core import setup, Extension, Command
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

class test_Pycluster(Command):
    "Run all of the tests for the package."

    user_options = []

    def initialize_options(self):
        shutil.copyfile(os.path.join('python','test','test_Cluster.py'),
                        'test_Cluster.py')

    def finalize_options(self):
        pass

    def run(self):
        import unittest
        import test_Cluster
        test_Cluster.TestCluster.module = 'Pycluster'
        suite = unittest.TestLoader().loadTestsFromModule(test_Cluster)
        runner = unittest.TextTestRunner(sys.stdout, verbosity = 2)
        runner.run(suite)


setup(name="Pycluster",
      version="1.52",
      description="The C Clustering Library",
      author="Michiel de Hoon",
      author_email="mdehoon 'AT' gsc.riken.jp",
      url="http://bonsai.ims.u-tokyo.ac.jp/~mdehoon/software/software.html",
      license="Python License",
      package_dir = {'Pycluster':'python'},
      packages = ['Pycluster'],
      ext_modules=[extension],
      cmdclass={"test" : test_Pycluster},
      )

## Process this file with automake to produce Makefile.in

DOCDIST = doc/cluster.pdf doc/cluster3.pdf doc/cluster.texinfo \
  doc/cluster3.texinfo doc/structure.eps doc/Makefile
WINDIST = windows/cluster.hhp windows/cluster.ico windows/cluster.iss \
  windows/main.c windows/gui.c windows/format.bmp windows/Makefile \
  windows/resources.h windows/resources.rc
MACDIST = mac/main.m mac/Controller.h mac/Controller.m mac/cluster.icns \
  mac/English.lproj/InfoPlist.strings \
  mac/English.lproj/AboutPanel.nib/classes.nib \
  mac/English.lproj/AboutPanel.nib/info.nib \
  mac/English.lproj/AboutPanel.nib/keyedobjects.nib \
  mac/English.lproj/FileFormatPanel.nib/classes.nib \
  mac/English.lproj/FileFormatPanel.nib/info.nib \
  mac/English.lproj/FileFormatPanel.nib/keyedobjects.nib \
  mac/English.lproj/MainMenu.nib/classes.nib \
  mac/English.lproj/MainMenu.nib/info.nib \
  mac/English.lproj/MainMenu.nib/keyedobjects.nib \
  mac/Cluster.xcodeproj/project.pbxproj mac/Makefile mac/Info.plist
PYTHONDIST = setup.py python/MANIFEST.python python/__init__.py \
  python/clustermodule.c python/test/README python/test/test_Cluster.py
PERLDIST = perl/Artistic.txt perl/MANIFEST.perl \
  Makefile.PL perl/Makefile.PL src/Makefile.PL \
  perl/Cluster.pm perl/Record.pm perl/Cluster.xs \
  perl/t/01_mean_median.t perl/t/02_tree.t \
  perl/t/10_kcluster.t perl/t/11_clusterdistance.t perl/t/12_treecluster.t \
  perl/t/13_somcluster.t perl/t/14_kmedoids.t perl/t/15_distancematrix.t \
  perl/t/16_pca.t \
  perl/examples/ex1_kcluster perl/examples/ex2_mean_median \
  perl/examples/ex3_kcluster perl/examples/ex4_somcluster \
  perl/examples/ex5_treecluster perl/examples/ex6_clusterdistance \
  perl/examples/ex7_distancematrix perl/examples/ex8_kmedoids
EXAMPLEDIST = example/example.c example/Makefile example/README
HTMLDIST = html/mac.py html/Makefile
DATADIST = data/README data/cyano.txt


if MOTIF
  MAYBE_X11 = X11
endif
SUBDIRS = $(MAYBE_X11) src

EXTRA_DIST = $(WINDIST) $(MACDIST) $(PYTHONDIST) $(PERLDIST) $(DOCDIST) \
  $(EXAMPLEDIST) $(HTMLDIST) $(DATADIST)

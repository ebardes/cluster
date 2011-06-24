## Process this file with automake to produce Makefile.in

bin_PROGRAMS = cluster
cluster_SOURCES = main.c command.c command.h data.c data.h cluster.c cluster.h 

if MOTIF
  cluster_SOURCES += gui.h
  AM_CPPFLAGS = -DHAVE_GUI
  LDADD = ../X11/libgui.a
endif

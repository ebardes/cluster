## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libgui.a
libgui_a_SOURCES = gui.c

HTML = ../html/*.html
IMAGE = ../html/images/*.png
FILEFORMAT = format.xpm
DOC = ../doc/cluster3.pdf

htmldir = $(prefix)/cluster/html
imagedir = $(prefix)/cluster/html/images
docdir = $(prefix)/cluster/doc
fileformatdir = $(prefix)/cluster

dist_html_DATA = $(HTML)
dist_image_DATA = $(IMAGE)
dist_doc_DATA = $(DOC)
dist_fileformat_DATA = $(FILEFORMAT)

AM_CPPFLAGS = -I../src $(X_CFLAGS) -DPREFIX=\"$(prefix)\"

OPTIONS = -DWINDOWS -DHAVE_GUI
SRCDIR = ../src
DOCDIR = ../doc
HTMLDIR = ../html

all: cluster.com clust95.com cluster.exe clust95.exe cluster.chm \
 $(DOCDIR)/cluster3.pdf

cluster.com: main.o command.o cluster.dll
	gcc -Wall -mconsole -O4 main.o command.o -L. -lcluster \
	  -o cluster.com

clust95.com: main95.o command.o cluster.dll
	gcc -Wall -mconsole -O4 main95.o command.o -L. -lcluster \
	  -o clust95.com

cluster.exe: gui.o resources.o cluster.dll
	gcc -Wall -mwindows -O4 \
	  gui.o resources.o -L. -lcluster -o cluster.exe

clust95.exe: gui95.o resources.o cluster.dll
	gcc -Wall -mwindows -O4 \
	  gui95.o resources.o -L. -lcluster -o clust95.exe

main.o: main.c
	gcc -mconsole -Wall -fno-strict-aliasing -c -O4 \
	  $(OPTIONS) -DUNICODE -I$(SRCDIR) main.c -o main.o

main95.o: main.c
	gcc -mconsole -Wall -fno-strict-aliasing -c -O4 \
	  $(OPTIONS) -I$(SRCDIR) main.c -o main95.o

gui.o: gui.c $(SRCDIR)/cluster.h
	gcc -mwindows -Wall -fno-strict-aliasing -c -O4 \
	  $(OPTIONS) -DUNICODE -I$(SRCDIR) gui.c

gui95.o: gui.c $(SRCDIR)/cluster.h
	gcc -mwindows -Wall -fno-strict-aliasing -c -O4 \
	  $(OPTIONS) -I$(SRCDIR) gui.c -o gui95.o

resources.o: resources.rc $(SRCDIR)/cluster.h
	windres -I$(SRCDIR) -i resources.rc -o resources.o

data.o: $(SRCDIR)/data.c
	gcc -mwindows -Wall -c -O4 $(OPTIONS) $(SRCDIR)/data.c

command.o: $(SRCDIR)/command.c
	gcc -mwindows -Wall -c -O4 $(OPTIONS) $(SRCDIR)/command.c

cluster.dll: cluster.o data.o
	gcc -Wall -O4 -shared -o cluster.dll -Wl,--export-all-symbols \
	  -L. cluster.o data.o

cluster.o: $(SRCDIR)/cluster.c
	gcc -c -O4 -Wall $(OPTIONS) $(SRCDIR)/cluster.c

clustersetup.exe: cluster.dll cluster.exe $(DOCDIR)/cluster3.pdf cluster.chm \
	  cluster.iss
	strip cluster.exe
	strip cluster.dll
	iscc cluster.iss

$(HTMLDIR)/index.html: $(DOCDIR)/cluster3.texinfo
	$(MAKE) -C $(HTMLDIR)

cluster.chm: cluster.hhp $(HTMLDIR)/index.html
	mv cluster.hhp $(HTMLDIR)
	cd $(HTMLDIR)
	-hhc $(HTMLDIR)/cluster.hhp
	mv $(HTMLDIR)/cluster.chm $(HTMLDIR)/cluster.hhp ../windows
	rm $(HTMLDIR)/toc.hhc
	cd ../windows

$(DOCDIR)/cluster3.pdf: $(DOCDIR)/cluster3.texinfo
	$(MAKE) -C $(DOCDIR)

clean:
	rm -f *.o *.a *.dll *.exe *.chm *.hhc

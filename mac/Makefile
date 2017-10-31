DOCDIR = ../doc
HTMLDIR = ../html

build/Default/Cluster.app: $(HTMLDIR)/html.helpindex $(HTMLDIR)/html\ idx $(DOCDIR)/cluster3.pdf
	xcodebuild -sdk macosx10.13
	rm -r build/Cluster.build

Cluster.pkg: build/Default/Cluster.app
	productbuild --component build/Default/Cluster.app /Applications Cluster.pkg

Cluster.tar: Cluster.pkg
	tar cvf Cluster.tar Cluster.pkg

$(HTMLDIR)/index.html: $(DOCDIR)/cluster3.texinfo
	$(MAKE) -C $(HTMLDIR)

$(HTMLDIR)/html.helpindex $(HTMLDIR)/html\ idx: $(HTMLDIR)/index.html
	hiutil -Caf $(HTMLDIR)/html.helpindex $(HTMLDIR)

$(DOCDIR)/cluster3.pdf: $(DOCDIR)/cluster3.texinfo
	$(MAKE) -C $(DOCDIR)

clean:
	rm -rf build Cluster.pkg Cluster.tar

docdir = ../doc

index.html: $(docdir)/cluster3.texinfo
	makeinfo --html $(docdir)/cluster3.texinfo
	python mac.py

distdir: .

clean:
	rm -f *.html

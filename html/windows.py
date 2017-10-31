#!python
import sys

filenames = sys.argv[1:]

for filename in filenames:
    inputfile = open(filename)
    text = inputfile.read()
    inputfile.close()
    # the attribute causes problems with the Windows help compiler.
    text = text.replace('<h2 class="contents-heading">', '<h2>')
    outputfile = open(filename,'w')
    outputfile.write(text)
    outputfile.close()

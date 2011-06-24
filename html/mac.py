#!python
# This script inserts a line in the header part of index.html that is required
# by Project Builder.
inputfile = open('index.html')
text = inputfile.read()
inputfile.close()
insert = text.find('</head>')

outputfile = open('index.html','w')
outputfile.write(text[:insert])
outputfile.write('<meta name="AppleTitle" content="Cluster 3.0 Help"></meta>\n')
outputfile.write(text[insert:])
outputfile.close()



all: xboxdrv.1 virtualkeyboard.1

%.1: %.xml Makefile
	echo "<!ENTITY xboxdrv_version \"$$(cat ../VERSION)\">" > xboxdrv.ent
	db2x_xsltproc --stylesheet man "$<" | db2x_manxml
	xmlto xhtml -o xboxdrv.html -m config.xsl xboxdrv.xml
	tidy -q -utf8 -indent -wrap 1000 -m xboxdrv.html/index.html

# EOF #

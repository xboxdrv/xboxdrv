all: xboxdrv.1

xboxdrv.1: xboxdrv.xml Makefile
	echo "<!ENTITY xboxdrv_version \"$$(cat ../VERSION)\">" > xboxdrv.ent
	db2x_xsltproc --stylesheet man "$<" | db2x_manxml
	xmlto xhtml -o xboxdrv.html -m config.xsl xboxdrv.xml
	tidy -utf8 -indent -wrap 1000 -m xboxdrv.html/index.html
#	docbook2x-texi --output-dir "../build/doc/" "$<"
#        MANWIDTH=80 man -l xboxdrv.1

# EOF #

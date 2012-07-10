# Project: Tsunami

BIN  = ../tsunami

all : $(BIN)

install : all
	mkdir -p /usr/share/tsunami
	mkdir -p /usr/share/tsunami/Data
	cp -f ../Data/hui_resources.txt    /usr/share/tsunami/Data
	cp -f ../Data/icon.*               /usr/share/tsunami/Data
	cp -f ../Data/license.txt          /usr/share/tsunami/Data
	cp -f ../Data/license_small.txt    /usr/share/tsunami/Data
	mkdir -p /usr/share/tsunami/Plugins
	cp -rf ../Plugins/*.kaba           /usr/share/tsunami/Plugins
	xdg-desktop-menu install ../Data/michisoft-tsunami.desktop
	xdg-mime install ../Data/michisoft-nami.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/Icons/32.png  application-michisoft-nami
	xdg-icon-resource install --context mimetypes --size  48 ../Data/Icons/48.png  application-michisoft-nami
	xdg-icon-resource install --context mimetypes --size  64 ../Data/Icons/64.png  application-michisoft-nami
	xdg-icon-resource install --context mimetypes --size 128 ../Data/Icons/128.png application-michisoft-nami
	#xdg-icon-resource install --context mimetypes ../Data/Icons/tsunami.svg application-michisoft-nami
	cp $(BIN) /usr/bin

all:
#	g++ -Wall -g -o amdexerciser-direct direct.cpp -Wno-write-strings -Wno-unused-function
#	g++ -Wall -g -o amdexerciser-cli cli.cpp -Wno-write-strings -Wno-unused-function
	gtk-builder-convert amdexerciser_app.glade amdexerciser_app.xml
	g++ -Wall -g -o amd_exerciser-app amdexerciser_main.cpp `pkg-config --cflags --libs --cflags gtk+-2.0 --libs --cflags libglade-2.0` -export-dynamic

clean:
	rm -f amd_exerciser-app amdexerciser_app.xml
	rm -f amdexerciser-cli amdexerciser-direct

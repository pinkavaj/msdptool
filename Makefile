
all: src doc

src: force_look
	make -C src

doc: force_look
	make -C doc doc
	doxygen

clean:
	make -C src clean
	make -C doc clean

install:
	make -C src install
	make -C doc install

force_look:
	true

wininst: all
	uname -s | grep "MINGW" && \
		wine "C:\\Program Files\\NSIS\\MakeNSIS.exe" install.nsi || \
		"C:\\Program Files\\NSIS\\MakeNSIS.exe" install.nsi


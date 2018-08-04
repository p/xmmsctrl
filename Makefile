PREFIX   = /usr/local

TARGET   = xmmsctrl
VERSION  = 1.5
DIRNAME  = $(shell basename $(PWD))

CC       = gcc
WARN     = -Wall -ansi -pedantic -Wshadow -Wmissing-prototypes
CFLAGS   = $(WARN) -O2 $(shell xmms-config --cflags) -DVERSION=\"$(VERSION)\"
LDFLAGS  = $(shell xmms-config --libs)

$(TARGET) : xmmsctrl.c
	$(CC) -o $(TARGET) xmmsctrl.c $(CFLAGS) $(LDFLAGS)
	strip xmmsctrl
	@if [ -e ${HOME}/bin/sstrip ]; then sstrip xmmsctrl; fi

install : $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin

clean :
	rm -f xmmsctrl core *~

distrib : clean
	rm -f ../$(TARGET)-$(VERSION).tar.gz
	cd .. && tar zcf $(TARGET)-$(VERSION).tar.gz $(DIRNAME)
	@if [ "$(TARGET)-$(VERSION)" != "$(DIRNAME)" ] ; then echo '*** WARNING ***' ; echo "Directory name doesn't match archive name!" ; fi
	chmod a+r ../$(TARGET)-$(VERSION).tar.gz

# The prefix is ignored if you are not the super user and
# xmmsctrl will be installed in your $HOME/bin
PREFIX   := /usr/local

TARGET   := xmmsctrl
VERSION  := 1.8
DIRNAME  := $(shell basename $(PWD))

CC       := gcc
WARN     := -Wall -Wshadow -Wmissing-prototypes -W
DEFS     := -DPRETTY_PRINT -D_GNU_SOURCE -DVERSION=\"$(VERSION)\"
CFLAGS   := $(WARN) -O2 $(shell xmms-config --cflags) $(DEFS)
LDFLAGS  := $(shell xmms-config --libs)

all : $(TARGET) HELP

$(TARGET) : xmmsctrl.c removefile.c
	$(CC) -o $(TARGET) xmmsctrl.c removefile.c $(CFLAGS) $(LDFLAGS)
	strip xmmsctrl

HELP : xmmsctrl
	./xmmsctrl > HELP || true

install : $(TARGET)
	@if [ $(shell whoami) = "root" ]; then\
	echo Installing xmmsctrl in $(PREFIX)/bin;\
	mkdir -p $(PREFIX)/bin;\
	install -m 755 $(TARGET) $(PREFIX)/bin;\
	else\
	echo Installing xmmsctrl in ${HOME}/bin;\
	mkdir -p ${HOME}/bin;\
	install -m 755 $(TARGET) ${HOME}/bin;\
	fi

clean :
	rm -f $(TARGET) HELP core *~

distrib : clean
	@if [ "$(TARGET)-$(VERSION)" != "$(DIRNAME)" ] ; then echo '*** WARNING ***' ; echo "Directory name doesn't match archive name!" ; fi
	rm -f ../$(TARGET)-$(VERSION).tar.gz
	cd .. && tar zcf $(TARGET)-$(VERSION).tar.gz $(DIRNAME) && chmod a+r $(TARGET)-$(VERSION).tar.gz

SRC = check_samfs.c
CFLAGS = -I../../samqfs/include
LDFLAGS = -L/opt/SUNWsamfs/lib -lfsmgmt -lcsn -L /usr/lib/fs/samfs -l samconf -L/usr/sfw/lib -lssl -lcurl\
 -R/opt/SUNWsamfs/lib -R/usr/lib/fs/samfs -R/usr/sfw/lib
BIN = check_samfs
prefix = /opt/csw

all: $(BIN)

$(BIN): src/$(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) src/$(SRC) -o $(BIN)

install:
	ginstall -d $(DESTDIR)/$(prefix)/nagios/libexec
	ginstall -m 755 $(BIN) $(DESTDIR)/$(prefix)/libexec/nagios-plugins

clean:
	rm -f $(BIN)

distclean: clean
	rm -rf autom4te.cache
	rm -f *.log
	rm -f config.status

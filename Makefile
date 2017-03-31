OS_REVISION := $(shell uname -r)
OS := $(shell uname -s)
ISA_TARGET := $(shell isainfo -k)
OS_ARCH := $(OS)_$(OS_REVISION)_$(ISA_TARGET)
OBJ_BASE := obj
OBJ_DIR := $(OBJ_BASE)/$(OS_ARCH)
prefix = /usr
exec_prefix = $(prefix)
libexecdir = ${exec_prefix}/libexec/nagios-plugins

CC=gcc
SRC = check_samfs.c
BIN = $(OBJ_DIR)/check_samfs

CFLAGS = -Iinclude
LDFLAGS = -z ignore -L./lib -lfsmgmt -L/usr/sfw/lib -lssl -lcurl -R/opt/SUNWsamfs/lib -R/usr/sfw/lib  -R/usr/lib/fs/samfs

D_CHECK_OBJDIR = \
	if [ ! -z "$(BIN)" ]; then \
		[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR); \
	fi
__dummy := $(shell $(D_CHECK_OBJDIR))

all: $(BIN)

$(BIN): src/$(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) src/$(SRC) -o $(BIN)

install:
	ginstall -d $(DESTDIR)/$(libexecdir)
	ginstall -m 755 $(BIN) $(DESTDIR)/$(libexecdir)

clean:
	rm -f $(BIN)

distclean: clean
	rm -rf autom4te.cache
	rm -f *.log
	rm -f config.status

OS_REVISION := $(shell uname -v | cut -d. -f1,2)
OS := $(shell uname -s)
ELFARCH := $(shell uname -p)
OS_ARCH := $(OS)_$(OS_REVISION)_$(ELFARCH)
OBJ_BASE := obj
OBJ_DIR := $(OBJ_BASE)/$(OS_ARCH)
prefix = /opt/SUNWsamfs/sbin
exec_prefix = $(prefix)
libexecdir = /opt/SUNWsamfs/util
IPSREPO ?= ./temp-repo
DESTDIR ?= ./proto

CC=/opt/solarisstudio12.4/bin/cc
# SRC = src/check_samfs.c src/ack_samfault.c src/sammgmt.c
BIN = $(OBJ_DIR)/check_samfs $(OBJ_DIR)/ack_samfault

CFLAGS = -I./include -errwarn=%all -errtags=yes -m32 
SAMLIBDIR = ./lib/SunOS_5.11_sparc
SAMLIBS = sam samut samcat samfs samapi fsmdb gen

SSLLIBPATH=$(if $(filter 5.10,$(OS_REVISION)),-L/usr/sfW/lib -lcsn -R/usr/sfW/lib)
LDFLAGS = -xs -z ignore -L$(SAMLIBDIR) $(foreach l,$(SAMLIBS),-l$l) $(SSLLIBPATH) -lssl -lcurl -R/opt/SUNWsamfs/lib -R/usr/lib/fs/samfs

D_CHECK_OBJDIR = \
	if [ ! -z "$(BIN)" ]; then \
		[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR); \
	fi
__dummy := $(shell $(D_CHECK_OBJDIR))

all: $(BIN)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/check_samfs:	 $(OBJ_DIR)/check_samfs.o $(OBJ_DIR)/sammgmt.o
	$(CC) $(LDFLAGS) $? -o $@

$(OBJ_DIR)/ack_samfault:	$(OBJ_DIR)/ack_samfault.o $(OBJ_DIR)/sammgmt.o
	$(CC) $(LDFLAGS) $? -o $@

install: $(BIN)
	ginstall -d $(DESTDIR)/$(libexecdir)
	$(foreach b, $(BIN), ginstall -m 755 $(b) $(DESTDIR)/$(libexecdir);)

pkg:
	pkgsend publish -d $(DESTDIR) -s $(IPSREPO) check_samfs.p5m
	pkgrecv -s $(IPSREPO) -a -d check_samfs.p5p check_samfs

clean:
	rm -f $(OBJ_DIR)/*
	rm -rf proto/*
	rm -f check_samfs.p5p

distclean: clean
	rm -rf autom4te.cache
	rm -f *.log
	rm -f config.status
	rm -f check_samfs.p5p

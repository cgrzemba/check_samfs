OS_REVISION := $(shell uname -r)
OS := $(shell uname -s)
ISA_TARGET := $(shell isainfo -k)
OS_ARCH := $(OS)_$(OS_REVISION)_$(ISA_TARGET)
OBJ_BASE := obj
OBJ_DIR := $(OBJ_BASE)/$(OS_ARCH)
prefix = /opt/SUNWsamfs/sbin
exec_prefix = $(prefix)
libexecdir = /opt/SUNWsamfs/util
IPSREPO = ~/solaris-userland/sparc/repo

CC=/opt/solarisstudio12.4/bin/cc
# SRC = src/check_samfs.c src/ack_samfault.c src/sammgmt.c
BIN = $(OBJ_DIR)/check_samfs $(OBJ_DIR)/ack_samfault

CFLAGS = -I./include -errwarn=%all -errtags=yes -m32 
SAMLIBDIR = ./lib/SunOS_5.11_sparcv9
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

$(OBJ_DIR)/check_samfs:	  $(OBJ_DIR)/sammgmt.o $(OBJ_DIR)/check_samfs.o
	$(CC) $(LDFLAGS) $? -o $@

$(OBJ_DIR)/ack_samfault:	$(OBJ_DIR)/sammgmt.o $(OBJ_DIR)/ack_samfault.o 
	$(CC) $(LDFLAGS) $? -o $@

install: $(BIN)
	ginstall -d $(DESTDIR)/$(libexecdir)
	$(foreach b, $(BIN), ginstall -m 755 $(b) $(DESTDIR)/$(libexecdir);)

pkg:
	pkgsend publish -s $(IPSREPO) check_samfs.p5m
	pkgrecv -s $(IPSREPO) -a -d check_samfs.p5p check_samfs

clean:
	rm -f $(OBJ_DIR)/*

distclean: clean
	rm -rf autom4te.cache
	rm -f *.log
	rm -f config.status
	rm -f check_samfs.p5p

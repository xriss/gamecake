PLAT= none

CC= gcc
CFLAGS= -O2 -Wall $(MYCFLAGS)
AR= ar rcu
RANLIB= ranlib
RM= rm -f
LIBS= $(MYLIBS) 
OUTLIB=lash.so

LDFLAGS= $(LIBS)

MYCFLAGS=
MYLDFLAGS=
MYLIBS=

OBJS = main.o md5.o crc32.o sha1.o rijndael.o

PLATS=linux macosx

all: 
	$(MAKE) none

build: lash

clean:
	$(RM) $(OBJS) $(OUTLIB)

lash: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTLIB) $(LDFLAGS)

none:
	@echo "Please choose a platform:"
	@echo "   $(PLATS)"

echo:
	@echo "PLAT = $(PLAT)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "MYCFLAGS = $(MYCFLAGS)"
	@echo "MYLDFLAGS = $(MYLDFLAGS)"
	@echo "MYLIBS = $(MYLIBS)"


linux:
	$(MAKE) build MYCFLAGS="-shared -fpic -I /usr/include/lua5.1/" MYLIBS=""

macosx:
	$(MAKE) build MACOSX_DEPLOYMENT_TARGET="10.3" MYCFLAGS="-fno-common -undefined dynamic_lookup -bundle -I /usr/local/include" MYLIBS=""

crc32.o: crc32.c
	$(CC) -c -o $@ $< $(CFLAGS)

md5.o: md5.c
	$(CC) -c -o $@ $< $(CFLAGS)

main.o: main.c
	$(CC) -c -o $@ $< $(CFLAGS)

sha1.o: sha1.c
	$(CC) -c -o $@ $< $(CFLAGS)

rijndael.o: rijndael.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: all $(PLATS) clean none 

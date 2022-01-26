
PLAT= generic

LUA= /opt/lua-5.2.1
POLARSSL= /opt/polarssl-1.2.5

T= polarssl.so
A= $(LUA)/src/libluapolarssl.a

COPT= -O2 -fPIC -DNDEBUG
CWARNS = -W -Wall -Wextra -pedantic \
	-Waggregate-return \
	-Wbad-function-cast \
	-Wcast-align \
	-Wcast-qual \
	-Wdeclaration-after-statement \
	-Wdisabled-optimization \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wpointer-arith \
	-Wshadow \
	-Wsign-compare \
	-Wstrict-prototypes \
	-Wundef \
	-Wwrite-strings \
	-Wunreachable-code

CFLAGS= $(COPT) $(CWARNS) $(MYCFLAGS) -I$(LUA)/src -I$(POLARSSL)/include
LIBS= -L$(POLARSSL)/library -lpolarssl $(MYLIBS)

MYCFLAGS=
MYLDFLAGS= -shared -Bdynamic
MYLIBS=

CC= gcc
MAKE_DLL= $(CC) $(MYLDFLAGS) -o
AR= ar rcu
RANLIB= ranlib
RM= rm -f
STRIP= strip

PLATS= generic osx

OBJS= luapolarssl.o
LDOBJS= $(OBJS)


default: $(PLAT)

all:	$(T)

a:	$(A)

$(T): $(OBJS)
	$(MAKE_DLL) $@ $(LDOBJS) $(LIBS)
	$(STRIP) $@

$(A): $(OBJS)
	$(AR) $@ $?
	$(RANLIB) $@


none:
	@echo "Please choose a platform:"
	@echo "	$(PLATS)"

generic:
	$(MAKE) all MYCFLAGS=

osx:
	$(MAKE) all MYCFLAGS= STRIP="echo strip" \
	MYLDFLAGS="-bundle -undefined dynamic_lookup /usr/lib/bundle1.o"

clean:
	$(RM) $(A) $(T) *.o


.PHONY: all $(PLATS) default a clean


luapolarssl.o: luapolarssl.c laes.c lbase64.c lcamellia.c ldes.c lhash.c \
 lmpi.c lpadlock.c lsession.c lssl.c lx509_cert.c lx509_crl.c common.h

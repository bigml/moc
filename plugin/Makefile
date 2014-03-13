MEVENT = 0

BASEDIR = ../../moon/
include $(BASEDIR)Make.env

UNAME := $(shell uname)
PREFIX=/usr/local

CFLAGS += -shared -fPIC
INC_MOON += -I../client -I../server -Iprivate
LIB_MOON += -levent

SOURCES = $(wildcard *.c)
LIB_PLUGIN = $(patsubst %.c, %.so, $(SOURCES))

all: $(LIB_PLUGIN)

moc_%.so: private/%_internal.c private/%_timer.c moc_%.c
	@echo "$(CC) -o $@"
ifeq ($(UNAME), Darwin)
	@$(CC) $(CFLAGS) -dynamiclib -undefined suppress -flat_namespace -o $@ $^ $(INC_MOON)
else
	@$(CC) $(CFLAGS) -o $@ $^ $(INC_MOON)
endif

install:
	install -d $(PREFIX)/lib
	install -m 0644 $(LIB_PLUGIN) $(PREFIX)/lib/

uninstall:
	rm -f $(PREFIX)/lib/$(LIB_PLUGIN)

clean:
	rm -f $(LIB_PLUGIN) *.o

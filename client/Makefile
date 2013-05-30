CC = gcc
EVENTLOOP = 0

CFLAGS = -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE
INC_MOC = -I./clearsilver

SOURCE1 = clearsilver/util/filter.c

SOURCES = $(wildcard clearsilver/util/*.c)
SOURCES += lerr.c packet.c mtrace.c    internal.c moc.c tcp.c
ifeq ($(EVENTLOOP), 1)
CFLAGS += -DEVENTLOOP
SOURCES += eloop.c mscli.c mcbk.c
endif


OBJS = $(patsubst %.c, %.o, $(SOURCES))
DEPEND = .depend

all: libmoc.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INC_MOC) > $@

libmoc.a: $(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

include $(DEPEND)
%.o:%.c
	@if [ "$<" = "$(SOURCE1)" ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INC_MOC); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INC_MOC)

install:

clean:
	rm -f $(OBJS) *.a

backup:
	@$(MAKE) clean
	@$(TAR) zcvf mocclient.tar.gz *

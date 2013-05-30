#don't use libmevent
MEVENT = 0

BASEDIR = ../../
include $(BASEDIR)Make.env

CFLAGS += -rdynamic
INC_MOON += -I../lib
LIB_MOON += -levent

SOURCE1 = cache.c
SOURCES = cache.c main.c mocd.c lerr.c net.c packet.c parse.c queue.c syscmd.c tcp.c
OBJS = $(patsubst %.c, %.o, $(SOURCES))
DEPEND = .depend

all: moc

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INC_MOON) > $@

moc: $(OBJS) $(DEPEND)
	$(CC) $(CFLAGS) $(OBJS) -o moc $(LIB_MOON)

include $(DEPEND)
%.o:%.c
	@if [ "$<" = "$(SOURCE1)" ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INC_MOON); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INC_MOON)

install:

clean:
	rm -f $(OBJS) moc

backup:
	@$(MAKE) clean
	@$(TAR) zcvf mocserver.tar.gz *

#LIB_ME = $(LIB_MOON:-lmevent=)
#INC_XX = $(INC_MOON:-I../../lib/mevent=)
#INC_ME = $(INC_XX:-I../../event/plugin=)

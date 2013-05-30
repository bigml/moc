CC = gcc

CFLAGS = -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE
INC_MOON = -I../

SOURCE1 = util/dict.c
SOURCES = $(wildcard util/*.c)
OBJS = $(patsubst %.c, %.o, $(SOURCES))
DEPEND = .depend

all: libcsutl.a

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) -MM $^ $(INC_MOON) > $@

libcsutl.a: $(OBJS) $(DEPEND)
	@rm -rf $@
	$(AR) rcs $@ $(OBJS)

include $(DEPEND)
%.o:%.c
	@if [ "$<" = "$(SOURCE1)" ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INC_MOON); fi
	@echo "$(CC) -c $<"
	@$(CC) $(CFLAGS) -c $< -o $@ $(INC_MOON)

install:

cflag:
	@echo "=== "$(CFLAGS)
	@echo "=== "$(INC_MOON)
	@echo "=== "$(LIB_MOON)

clean:
	rm -f $(OBJS) *.a

backup:
	@$(MAKE) clean
	@$(TAR) zcvf libmoc.tar.gz *

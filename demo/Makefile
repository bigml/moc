INCS = -I../client -I../client/clearsilver -I../server
LIBS = -L../client -lmoc -lpthread -lrt

SOURCES = $(wildcard *.c)
BINARY = $(patsubst %.c, %, $(SOURCES))

CFLAGS = -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE

all: $(BINARY)

%:%.c
	@if [ "$<" = `ls *.c|awk '{print $1}'|sed -n '1p'` ]; then \
		echo "=== "$(CFLAGS); \
		echo "=== "$(INCS); \
		echo "=== "$(LIBS); fi
	@echo "$(CC) -o $@"
	@$(CC) $(CFLAGS) $< -o $@ $(INCS) ${LIBS}

install:

clean:
	rm -f $(BINARY)

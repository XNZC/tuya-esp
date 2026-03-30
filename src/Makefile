CC:=gcc
CFLAGS:= -g -Wall -Wextra
BIN:=tuya-esp
LIB:=/usr/lib
EXTRA:= -llink_core -lmiddleware_implementation -lplatform_port -lutils_modules -largp -lubus -lubox -lblobmsg_json -luci
SRC:=$(wildcard *.c)
OBJS:=$(SRC:.c=.o)
.PHONY: all clean
all: $(BIN)
$(BIN): $(OBJS)
	$(CC) -L $(LIB) -o $@ $^ $(EXTRA)
%.o: %.c
	$(CC) $(CFLAGS) -I. -c $< -o $@
clean:
	rm -f $(BIN) $(OBJS)
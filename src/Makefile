	CFLAGS:= -std=c11 -pedantic  -pedantic-errors -pthread -g -Wall  -Wextra -D_POSIX_C_SOURCE=200809L -Werror -fsanitize=address -Ilib/include -lcrypto
STEGOBMP_CLI:= stegobmp.elf
LIB_OBJS:= build/parse_args.o
MAIN_OBJ:=build/main.o
.PHONY: all clean

all: $(STEGOBMP_CLI)

$(STEGOBMP_CLI): $(LIB_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(MAIN_OBJ) -o $(STEGOBMP_CLI)

clean:
	- rm -rf $(STEGOBMP_CLI) build/*.o build

build/%.o: lib/%.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@

build/main.o: main.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@


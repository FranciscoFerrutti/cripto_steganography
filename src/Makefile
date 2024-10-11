CFLAGS:= -std=c11 -pedantic -pedantic-errors -pthread -g -Wall -Wextra -D_POSIX_C_SOURCE=200809L -Werror -fsanitize=address -Ilib/include
LDFLAGS:= -lcrypto
STEGOBMP_CLI:= stegobmp.elf
LIB_SRCS:= lib/parse_args.c lib/embed.c lib/bmp_utils.c lib/encryption.c
LIB_OBJS:= $(LIB_SRCS:lib/%.c=build/%.o)
MAIN_OBJ:= build/main.o
.PHONY: all clean

all: $(STEGOBMP_CLI)

$(STEGOBMP_CLI): $(LIB_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(MAIN_OBJ) -o $@ $(LDFLAGS)

clean:
	- rm -rf $(STEGOBMP_CLI) build/*.o build

build/%.o: lib/%.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@

build/main.o: main.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@
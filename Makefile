CC = clang

PWD = $(shell pwd)
CFLAGS = -I$(PWD)/thirdparty/ -Wall -Wextra

comma := ,
empty :=
space := $(empty) $(empty)
FLAGS_LIST = $(subst $(space),$(comma),$(CFLAGS))
a := $(file > .clangd, CompileFlags:)
b := $(file >> .clangd, 	Add: [$(FLAGS_LIST)])

.PHONY: default clean dump gen parse bk all quick docs

default: build/bk

docs:
	doxygen

bk: ./build/bk

debug: ./build/bk_debug

gen: ./build/bk ./examples/people.h ./examples/dump_people.c ./examples/.bk.conf
	./build/bk --config-path ./examples/.bk.conf

quick: ./build/quick

dump: ./build/dump_people

parse: ./build/parse_people

schema_ext: ./build/bk_ext

all: quick dump parse schema_ext

gen/bk_ext.h: build/bk
	./build/bk --gen-ext ./bk.c ./gen/bk_ext.h -dW no-output

build/bk: bk.c ./thirdparty/stb_c_lexer.h
	$(CC) $(CFLAGS) bk.c -o ./build/bk

build/bk_debug: bk.c ./thirdparty/stb_c_lexer.h
	$(CC) $(CFLAGS) bk.c -g -DDEBUG -o ./build/bk_debug

build/quick: ./examples/people.h ./examples/quick.c gen
	$(CC) $(CFLAGS) -g ./examples/quick.c -o ./build/quick

build/dump_people: ./examples/people.h ./examples/dump_people.c gen
	$(CC) $(CFLAGS) -g ./examples/dump_people.c -o ./build/dump_people

build/parse_people: ./examples/people.h ./examples/parse_people.c gen ./thirdparty/cJSON.c ./thirdparty/cJSON.h
	$(CC) $(CFLAGS) -g ./examples/parse_people.c ./thirdparty/cJSON.c -o ./build/parse_people

build/bk_ext: gen/bk_ext.h
	$(CC) $(CFLAGS) ./examples/bk_ext.c -o ./build/bk_ext

clean:
	rm -f ./examples/*.bk.h
	rm -f ./examples/derives.h
	rm -r ./build/
	mkdir -p build
	rm -r ./gen
	mkdir -p gen

PWD = $(shell pwd)
CFLAGS = -I$(PWD)/thirdparty/ -Wunused-parameter

comma := ,
empty :=
space := $(empty) $(empty)
FLAGS_LIST = $(subst $(space),$(comma),$(CFLAGS))
a := $(file > .clangd, CompileFlags:)
b := $(file >> .clangd, 	Add: [$(FLAGS_LIST)])

.PHONY: default clean dump gen parse bk

default: build/bookkeeper_gen

bk: ./build/bookkeeper_gen

debug: ./build/bookkeeper_gen_debug

gen: ./build/bookkeeper_gen ./examples/people.h ./examples/dump_people.c
	./build/bookkeeper_gen ./examples ./gen

dump: ./build/dump_people

parse: ./build/parse_people

schema_ext: ./build/bookkeeper_gen_ext

build/bookkeeper_gen: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h
	clang $(CFLAGS) bookkeeper_gen.c -o ./build/bookkeeper_gen

build/bookkeeper_gen_debug: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h
	clang $(CFLAGS) bookkeeper_gen.c -g -DDEBUG -o ./build/bookkeeper_gen_debug

build/dump_people: ./examples/people.h ./examples/dump_people.c gen
	clang $(CFLAGS) -g ./examples/dump_people.c -o ./build/dump_people

build/parse_people: ./examples/people.h ./examples/parse_people.c gen ./thirdparty/cJSON.c ./thirdparty/cJSON.h
	clang $(CFLAGS) -g ./examples/parse_people.c ./thirdparty/cJSON.c -o ./build/parse_people

build/bookkeeper_gen_ext: ./bookkeeper_gen.c ./bookkeper_gen_ext.h ./thirdparty/stb_c_lexer.h
	clang $(CFLAGS) ./examples/bookkeeper_gen_ext.c -o ./build/bookkeeper_gen_ext

clean:
	rm -r ./build/
	mkdir -p build
	rm -r ./gen
	mkdir -p gen

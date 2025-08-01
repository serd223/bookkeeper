CFLAGS = -I./thirdparty/

comma := ,
empty :=
space := $(empty) $(empty)
FLAGS_LIST = $(subst $(space),$(comma),$(CFLAGS))
a := $(file > .clangd, CompileFlags:)
b := $(file >> .clangd, 	Add: [$(FLAGS_LIST)])

default: build/bookkeeper_gen build/bookkeeper_gen_debug

bk_gen: ./build/bookkeper_gen

dump: ./build/dump_people

build:
	mkdir -p ./build

gen:
	mkdir -p ./gen

build/bookkeeper_gen: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h build
	clang $(CFLAGS) bookkeeper_gen.c -o ./build/bookkeeper_gen

build/bookkeeper_gen_debug: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h build
	clang $(CFLAGS) bookkeeper_gen.c -g -DDEBUG -o ./build/bookkeeper_gen_debug

gen/bookkeeper.c: ./build/bookkeeper_gen gen
	./build/bookkeeper_gen ./examples && mv ./bookkeeper.c ./gen/ && mv ./bookkeeper.h ./gen/

gen/bookkeeper.h: ./gen/bookkeeper.c gen

build/dump_people: ./examples/people.h ./examples/dump_people.c ./gen/bookkeeper.c ./gen/bookkeeper.h build
	clang $(CFLAGS) ./examples/dump_people.c -o ./build/dump_people

clean: build gen
	rm -r ./build/ && mkdir -p build && rm -r ./gen && mkdir -p gen

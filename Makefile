CFLAGS = -I./thirdparty/

comma := ,
empty :=
space := $(empty) $(empty)
FLAGS_LIST = $(subst $(space),$(comma),$(CFLAGS))
a := $(file > .clangd, CompileFlags:)
b := $(file >> .clangd, 	Add: [$(FLAGS_LIST)])

.PHONY: default clean dump bk_gen

default: build/bookkeeper_gen build/bookkeeper_gen_debug

gen: ./build/bookkeeper_gen

dump: ./build/dump_people

build/bookkeeper_gen: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h
	clang $(CFLAGS) bookkeeper_gen.c -o ./build/bookkeeper_gen

build/bookkeeper_gen_debug: bookkeeper_gen.c ./thirdparty/stb_c_lexer.h
	clang $(CFLAGS) bookkeeper_gen.c -g -DDEBUG -o ./build/bookkeeper_gen_debug

gen/bookkeeper.c gen/bookkeeper.h: build/bookkeeper_gen ./examples/people.h
	./build/bookkeeper_gen ./examples
	mv ./bookkeeper.c ./gen/
	mv ./bookkeeper.h ./gen/

build/dump_people: ./examples/people.h ./examples/dump_people.c gen/bookkeeper.c gen/bookkeeper.h
	clang $(CFLAGS) ./examples/dump_people.c -o ./build/dump_people

clean:
	rm -r ./build/ && mkdir -p build && rm -r ./gen && mkdir -p gen

default: bookkeeper_gen bookkeeper_gen_debug

bookkeeper_gen: bookkeeper_gen.c stb_c_lexer.h
	clang bookkeeper_gen.c -o bookkeeper_gen

bookkeeper_gen_debug: bookkeeper_gen.c stb_c_lexer.h
	clang bookkeeper_gen.c -g -DDEBUG -o bookkeeper_gen_debug

bookkeeper.c: bookkeeper_gen
	./bookkeeper_gen ./examples

dump_people: ./examples/people.h ./examples/dump_people.c bookkeeper.c
	clang ./examples/dump_people.c -o dump_people

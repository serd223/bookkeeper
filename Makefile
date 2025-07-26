default: analyzer analyzer_debug

analyzer: analyzer.c stb_c_lexer.h
	clang analyzer.c -o analyzer

analyzer_debug: analyzer.c stb_c_lexer.h
	clang analyzer.c -g -DDEBUG -o analyzer_debug

bookkeeper.c: analyzer
	./analyzer ./examples

dump_people: ./examples/people.h ./examples/dump_people.c bookkeeper.c
	clang ./examples/dump_people.c -o dump_people

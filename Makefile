all: analyzer analyzer_debug

analyzer: analyzer.c stb_c_lexer.h
	clang analyzer.c -o analyzer -I.

analyzer_debug: analyzer.c stb_c_lexer.h
	clang analyzer.c -g -DDEBUG -o analyzer_debug -I.

flex --header-file=scanner.h -o scanner.cpp lexico.l
bison -o parser.cpp --defines=parser.h sintactico.y
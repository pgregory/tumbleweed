CC = gcc
LIBS = -lm -ledit
default:
	@cd source && $(CC) -c memory.c names.c news.c interp.c primitive.c filein.c lex.c parser.c unixio.c st.c initial.c tty.c
	@cd source && $(CC) memory.o names.o news.o interp.o primitive.o filein.o lex.o parser.o unixio.o st.o tty.o -o ../st $(LIBS)
	@cd source && $(CC) memory.o names.o news.o interp.o primitive.o filein.o lex.o parser.o unixio.o initial.o tty.o -o ../initial $(LIBS)
	@cd bootstrap && ../initial basic.st mag.st collect.st file.st mult.st tty.st
	@mv bootstrap/systemImage .

clean:
	rm -f source/*.o st initial systemImage

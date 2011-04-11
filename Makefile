CC = gcc -g
CFLAGS += $(shell pkg-config --cflags libffi)
LIBS = -lm -ledit
LDFLAGS += $(shell pkg-config --libs libffi)
default:
	@cd source && $(CC) $(CFLAGS) -c memory.c names.c news.c interp.c primitive.c filein.c lex.c parser.c unixio.c st.c initial.c sysprimitive.c ffiprimitive.c
	@cd source && $(CC) memory.o names.o news.o interp.o primitive.o filein.o lex.o parser.o unixio.o st.o sysprimitive.o ffiprimitive.o -o ../st $(LIBS) $(LDFLAGS) 
	@cd source && $(CC) memory.o names.o news.o interp.o primitive.o filein.o lex.o parser.o unixio.o initial.o sysprimitive.o ffiprimitive.o -o ../initial $(LIBS) $(LDFLAGS) 
	@cd bootstrap && ../initial basic.st mag.st collect.st file.st mult.st tty.st ffi.st
	@mv bootstrap/systemImage .

clean:
	rm -f source/*.o st initial systemImage

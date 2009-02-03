CC=gcc
OPT=-Wall -g -lm
OUTPUT=a3
DEBUG=debug
SRC=util.c main.c 
OBJ=util.o main.o
CTAGS=ctags *

default: GENHEADER DEBUG NODEBUG
	$(CTAGS)

DEBUG: $(OBJ)
	$(CC) $(OPT) -DDEBUG -o $(DEBUG) $(SRC) 

NODEBUG: $(OBJ)
	$(CC) $(OPT) -o $(OUTPUT) $(OBJ) 

GENHEADER: $(SRC)
	./genheader.sh $(SRC)

$(OBJ): header.h

.PHONY : clean 
clean:
	-rm $(OBJ) $(DEBUG) $(OUTPUT)

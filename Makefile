SRC = parse_xml.c
OBJ = parse_xml

CC = gcc
CFLAGS = -g -o

all:
	$(CC) $(CFLAGS) $(OBJ) $(SRC)
clean:
	rm -f $(OBJ)

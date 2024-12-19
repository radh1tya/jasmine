CC = cc
INPUT = main.c
PKG = pkg-config --libs --cflags $(GTK)
GTK = gtk+-3.0
OUTPUT = jasmine

install:
	$(CC) $(INPUT) -o $(OUTPUT) `$(PKG)`

remove:
	$(CC) $(OUTPUT)

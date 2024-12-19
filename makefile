CC = cc
INPUT = main.c
PKG = pkg-config --libs --cflags $(GTK)
GTK = gtk+-3.0
OUTPUT = bloom

install:
	$(CC) $(INPUT) -o $(OUTPUT) `$(PKG)`


CC = cc
INPUT = main.c
PKG = pkg-config --libs --cflags $(GTK)
GTK = gtk+-3.0
OUTPUT = jasmine

all: $(OUTPUT)

$(OUTPUT): $(INPUT)
    $(CC) $(INPUT) -o $(OUTPUT) `$(PKG)`

configure:
    @echo "ga perlu configure"

clean:
    rm -f $(OUTPUT)

check:
    @echo "ga perlu check"

distcheck:
    @echo "ga perlu distcheck"

.PHONY: all install clean check distcheck configure

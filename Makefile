CFLAGS = -Wall -O2
LIBS = -lm
DBFLAGS = -g -Wall
TARGET = src/cbase

cbase: src/cbase.c
	$(CC) src/cbase.c -o $(TARGET) $(CFLAGS) $(LIBS)
	strip $(TARGET)

debug: src/cbase.c
	$(CC) src/cbase.c -o $(TARGET) $(LIBS) $(DBFLAGS)

install: src/cbase
	mkdir -p $(DESTDIR)/bin
	cp $(TARGET) $(DESTDIR)/bin

clean:
	rm -rf $(TARGET)

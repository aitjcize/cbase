CFLAGS = -Wall -O2
LIBS = -lm
DBFLAGS = -g -Wall
TARGET = cbase

cbase: cbase.c
	$(CC) cbase.c -o $(TARGET) $(CFLAGS) $(LIBS)
	strip $(TARGET)

debug: cbase.c
	$(CC) cbase.c -o $(TARGET) $(LIBS) $(DBFLAGS)

install:
	cp cbase ~/bin

clean:
	rm -rf cbase

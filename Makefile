CC=clang

TARGET=solir

debug:CFLAGS =

all: debug

debug: $(TARGET)
	
$(TARGET):
	$(CC) -o $(TARGET) $(CFLAGS) src/main.c

clean:
	- rm -f $(TARGET)

.PHONY: all debug clean

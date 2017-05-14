CC = gcc

CFLAGS  = -framework Carbon

TARGET = aliasToSymlink

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c getTrueName.c

clean:
	$(RM)  $(TARGET)


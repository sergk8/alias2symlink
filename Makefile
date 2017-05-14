CC = gcc

CFLAGS  = -framework Carbon

TARGET = aliasToSymlink

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM)  $(TARGET)


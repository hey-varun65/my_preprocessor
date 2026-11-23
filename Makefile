CC = gcc
CFLAGS = -Wall -Wextra -g

TARGET = preprocessor
SOURCE = preproccesor.c

all: $(TARGET)

$(TARGET): $(SOURCE) preprocessor.h
        $(CC) $(CFLAGS) $(SOURCE) -o $(TARGET)

clean:
        rm -f $(TARGET) *.i

CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -Iinclude

TARGET = hw-01_bmp
OBJDIR = obj
SRCDIR = src

SOURCES = $(SRCDIR)/main.c $(SRCDIR)/bmp.c
OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/bmp.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/main.o: $(SRCDIR)/main.c include/bmp.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/bmp.o: $(SRCDIR)/bmp.c include/bmp.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean

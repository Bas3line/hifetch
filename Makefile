CC = gcc
CFLAGS = -O3 -march=x86-64 -mtune=generic -flto -ffast-math -funroll-loops \
         -fomit-frame-pointer -finline-functions -DNDEBUG -std=c99 \
         -w -Iinclude
LDFLAGS = -Wl,-O1 -Wl,--as-needed -Wl,--strip-all -lpthread -lm
STATIC_LDFLAGS = -static -Wl,-O1 -Wl,--strip-all -lpthread -lm
COMPAT_CFLAGS = -O2 -march=x86-64 -mtune=generic -std=c99 -w -Iinclude

TARGET = hifetch
HTOP_TARGET = hitop
SRCDIR = src
INCDIR = include
BINDIR = bin

SYSFETCH_SOURCES = $(SRCDIR)/main.c $(SRCDIR)/sysinfo.c $(SRCDIR)/ascii.c $(SRCDIR)/utils.c \
                   $(SRCDIR)/simd_optimizations.c $(SRCDIR)/security.c
HTOP_SOURCES = $(SRCDIR)/hitop.c $(SRCDIR)/process.c $(SRCDIR)/utils.c

.PHONY: all clean run hitop run-hitop install-hitop static compat

all: directories $(TARGET)

directories:
	@mkdir -p $(BINDIR)

$(TARGET): directories $(SYSFETCH_SOURCES)
	$(CC) $(CFLAGS) $(SRCDIR)/main.c $(SRCDIR)/sysinfo.c $(SRCDIR)/ascii.c $(SRCDIR)/utils.c $(SRCDIR)/simd_optimizations.c $(SRCDIR)/security.c -o $(BINDIR)/$(TARGET) $(LDFLAGS)
	strip --strip-all $(BINDIR)/$(TARGET)

static: directories $(SYSFETCH_SOURCES)
	$(CC) $(COMPAT_CFLAGS) $(SRCDIR)/main.c $(SRCDIR)/sysinfo.c $(SRCDIR)/ascii.c $(SRCDIR)/utils.c $(SRCDIR)/simd_optimizations.c $(SRCDIR)/security.c -o $(BINDIR)/$(TARGET)-static $(STATIC_LDFLAGS)
	strip --strip-all $(BINDIR)/$(TARGET)-static

compat: directories $(SYSFETCH_SOURCES)
	$(CC) $(COMPAT_CFLAGS) $(SRCDIR)/main.c $(SRCDIR)/sysinfo.c $(SRCDIR)/ascii.c $(SRCDIR)/utils.c $(SRCDIR)/simd_optimizations.c $(SRCDIR)/security.c -o $(BINDIR)/$(TARGET) $(LDFLAGS)
	strip --strip-all $(BINDIR)/$(TARGET)

$(HTOP_TARGET): directories $(HTOP_SOURCES)
	$(CC) $(CFLAGS) $(HTOP_SOURCES) -o $(BINDIR)/$(HTOP_TARGET) $(LDFLAGS) -lncurses
	strip --strip-all $(BINDIR)/$(HTOP_TARGET)

run: $(TARGET)
	$(BINDIR)/$(TARGET)

run-hitop: $(HTOP_TARGET)
	$(BINDIR)/$(HTOP_TARGET)

install-hitop: $(HTOP_TARGET)
	sudo cp $(BINDIR)/$(HTOP_TARGET) /usr/local/bin/

clean:
	rm -rf $(BINDIR)

bench: $(TARGET)
	@echo "Benchmarking hifetch vs fastfetch:"
	@echo "HIFETCH (5 runs):"
	@for i in {1..5}; do time $(BINDIR)/$(TARGET) > /dev/null; done
	@echo "FastFetch (5 runs):"
	@for i in {1..5}; do time fastfetch > /dev/null 2>&1 || echo "fastfetch not found"; done
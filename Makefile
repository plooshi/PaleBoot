INCLDIRS = -I./incl
LIBDIRS ?= -L/usr/local/lib -L/usr/local/lib64
GASTER_SRC = ./deps/gaster/gaster.c ./deps/gaster/lzfse.c
SRC = $(wildcard src/*)
OBJDIR = obj
OBJS = $(patsubst src/%,$(OBJDIR)/%,$(SRC:.c=.o))
GASTER_OBJS = ./$(OBJDIR)/gaster/gaster.o ./$(OBJDIR)/gaster/lzfse.o

ifeq ($(shell uname),Darwin)
USBLIB_FLAGS=
CFLAGS ?= -O2
LIBS = -lm -lplist-2.0 -limobiledevice-glue-1.0 -lirecovery-1.0 -limobiledevice-1.0 -lusbmuxd-2.0 -lcurl
LDFLAGS ?=
CC := xcrun -sdk macos clang
else
USBLIB_FLAGS=-DHAVE_LIBUSB
CFLAGS ?= -O2 -static
CURL_LIBS=-lcurl -lnghttp2 -lzstd -lbrotlicommon -lbrotlidec -lz
LIBS = -lcrypto -lssl -lm -lplist-2.0 -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0 -limobiledevice-1.0 -lusbmuxd-2.0 -lcurl $(CURL_LIBS)
LDFLAGS ?= -fuse-ld=lld
CC := clang
endif

all: dirs submodules gaster $(OBJS) $(GASTER_OBJS) PaleBoot

dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/gaster

submodules:
	@git submodule update --init --remote --recursive || true

gaster:
ifeq (,$(wildcard deps/gaster/gaster.mk))
	@cp gaster.mk deps/gaster
	@make -f gaster.mk -C deps/gaster -s
endif

clean:
	@rm -rf PaleBoot obj

PaleBoot: $(OBJS) $(GASTER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBDIRS) $(LIBS) $(INCLDIRS) $(OBJS) $(GASTER_OBJS) -o $@

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLDIRS) -c -o $@ $<

$(OBJDIR)/gaster/%.o: deps/gaster/%.c
	$(CC) $(CFLAGS) $(INCLDIRS) $(USBLIB_FLAGS) -DVERSION=\"1.0-paleboot1\" -c -o $@ $<

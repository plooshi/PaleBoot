CC := clang
CFLAGS ?= -O2 -static
LIBS = -lcrypto -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0
INCLDIRS = -I./incl
LIBDIRS ?= -L/usr/local/lib -L/usr/local/lib64
LDFLAGS ?= -fuse-ld=lld
GASTER_FILES = ./deps/gaster/gaster.c ./deps/gaster/lzfse.c

ifeq ($(shell uname),Darwin)
USBLIB_FLAGS=
else
USBLIB_FLAGS=-DHAVE_LIBUSB
endif

all: submodules gaster paleboot

submodules:
	git submodule update --init --remote --recursive || true

gaster:
	cp gaster.mk deps/gaster
	make -f gaster.mk -C deps/gaster

paleboot:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(GASTER_FILES) $(LIBDIRS) $(INCLDIRS) $(USBLIB_FLAGS) paleboot.c -o PaleBoot -DVERSION=\"1.0-paleboot1\"
INCLDIRS = -I./incl
LIBDIRS ?= -L/usr/local/lib -L/usr/local/lib64
GASTER_FILES = ./deps/gaster/gaster.c ./deps/gaster/lzfse.c

ifeq ($(shell uname),Darwin)
USBLIB_FLAGS=
CFLAGS ?= -O2
LIBS = -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0
CC := xcrun -sdk macos clang
else
USBLIB_FLAGS=-DHAVE_LIBUSB
CFLAGS ?= -O2 -static
LIBS = -lcrypto -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0
LDFLAGS ?= -fuse-ld=lld
CC := clang
endif

all: submodules gaster paleboot

submodules:
	git submodule update --init --remote --recursive || true

gaster:
	cp gaster.mk deps/gaster
	make -f gaster.mk -C deps/gaster

paleboot:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(LIBDIRS) $(INCLDIRS) $(USBLIB_FLAGS) src/paleboot.c $(GASTER_FILES) -o ./PaleBoot -DVERSION=\"1.0-paleboot1\"
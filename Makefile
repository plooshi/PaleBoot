CC ?= clang
CFLAGS ?= -O2 -static 
LIBS = -lcrypto -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0
INCLDIRS = -I./incl
LIBDIRS ?= -L/usr/local/lib -L/usr/local/lib64
LDFLAGS ?= -fuse-ld=lld
GASTER_FILES = ./deps/gaster/gaster.c ./deps/gaster/lzfse.c

all: gaster paleboot

gaster:
	cd deps/gaster
	cat Makefile | sed 's/-o gaster/-o libgaster.a/' | tee Makefile > /dev/null
	cat gaster.c | sed 's/main/gaster_main/' | sed 's/static //' | sed 's/unsigned usb_timeout;/unsigned usb_timeout = 5;/' | tee gaster.c > /dev/null
	xxd -iC payload_A9.bin payload_A9.h
	xxd -iC payload_notA9.bin payload_notA9.h
	xxd -iC payload_notA9_armv7.bin payload_notA9_armv7.h
	xxd -iC payload_handle_checkm8_request.bin payload_handle_checkm8_request.h
	xxd -iC payload_handle_checkm8_request_armv7.bin payload_handle_checkm8_request_armv7.h
	touch .compiled

paleboot:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(GASTER_FILES) $(LIBDIRS) $(INCLDIRS) paleboot.c -o PaleBoot -DHAVE_LIBUSB -DVERSION=\"1.0-paleboot1\"
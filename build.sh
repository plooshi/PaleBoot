#!/usr/bin/env bash

git submodule update --init --recursive --remote || true
cd deps/gaster
if [ ! -f ".compiled" ]; then
    cat Makefile | sed 's/-o gaster/-o libgaster.a/' | tee Makefile > /dev/null
    cat gaster.c | sed 's/main/gaster_main/' | sed 's/static //' | tee gaster.c > /dev/null
    xxd -iC payload_A9.bin payload_A9.h
	xxd -iC payload_notA9.bin payload_notA9.h
	xxd -iC payload_notA9_armv7.bin payload_notA9_armv7.h
	xxd -iC payload_handle_checkm8_request.bin payload_handle_checkm8_request.h
	xxd -iC payload_handle_checkm8_request_armv7.bin payload_handle_checkm8_request_armv7.h
    touch .compiled
fi
cd ../..
clang -fuse-ld=lld -L./libs -I./incl -L/usr/local/lib -L/usr/local/lib64 -lcrypto -lusb-1.0 -limobiledevice-glue-1.0 -lirecovery-1.0 \
    paleboot.c ./deps/gaster/gaster.c ./deps/gaster/lzfse.c -O2 -o PaleBoot -DHAVE_LIBUSB -DVERSION=\"1.0-paleboot1\"
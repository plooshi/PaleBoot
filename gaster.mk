all: gaster

gaster:
	cat Makefile | sed 's/-o gaster/-o libgaster.a/' | tee Makefile > /dev/null
	cat gaster.c | sed 's/main/gaster_main/' | sed 's/static //' | sed 's/unsigned usb_timeout;/unsigned usb_timeout = 5;/' | tee gaster.c > /dev/null
	xxd -iC payload_A9.bin payload_A9.h
	xxd -iC payload_notA9.bin payload_notA9.h
	xxd -iC payload_notA9_armv7.bin payload_notA9_armv7.h
	xxd -iC payload_handle_checkm8_request.bin payload_handle_checkm8_request.h
	xxd -iC payload_handle_checkm8_request_armv7.bin payload_handle_checkm8_request_armv7.h
#ifndef _GASTER_H
#define _GASTER_H
#include <libusb-1.0/libusb.h>
#include <stdbool.h>

typedef struct {
	uint16_t vid, pid;
	int usb_interface;
	struct libusb_context *context;
	struct libusb_device_handle *device;
} usb_handle_t;

bool gaster_checkm8(usb_handle_t *handle);
bool gaster_reset(usb_handle_t *handle);
#endif /* _GASTER_H */
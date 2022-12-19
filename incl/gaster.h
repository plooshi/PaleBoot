#ifndef _GASTER_H
#define _GASTER_H

#include <usb.h>
#include <stdbool.h>

typedef struct {
	uint16_t vid, pid;
#ifdef USE_LIBUSB
	int usb_interface;
	struct libusb_context *context;
	struct libusb_device_handle *device;
#else
	io_service_t serv;
	IOUSBDeviceInterface320 **device;
	CFRunLoopSourceRef async_event_source;
	IOUSBInterfaceInterface300 **interface;
#endif
} usb_handle_t;

bool gaster_checkm8(usb_handle_t *handle);
bool gaster_reset(usb_handle_t *handle);

#endif /* _GASTER_H */
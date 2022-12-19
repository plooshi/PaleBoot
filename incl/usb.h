#ifndef _USB_H
#define _USB_H

// deps
#if defined(HAVE_LIBUSB) || defined(_WIN32) || defined(__linux__)

#define USE_LIBUSB
#include <libusb-1.0/libusb.h>
#else
#define USE_IOKIT
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#endif /* defined(HAVE_LIBUSB) || defined(_WIN32) || defined(__linux__) */

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

#endif /* _USB_H */
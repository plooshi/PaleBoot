#ifndef _USB_H
#define _USB_H

#if defined(HAVE_LIBUSB) || defined(_WIN32) || defined(__linux__)
#define USE_LIBUSB
#include <libusb-1.0/libusb.h>
#else
#define USE_IOKIT
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <CommonCrypto/CommonCrypto.h>
#endif /* defined(HAVE_LIBUSB) || defined(_WIN32) || defined(__linux__) */
#endif /* _USB_H */
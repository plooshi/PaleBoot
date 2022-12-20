#ifndef _GASTER_H
#define _GASTER_H

// built in
#include "usb.h"

// std c library
#include <stdbool.h>

bool gaster_checkm8(usb_handle_t *handle);
bool gaster_reset(usb_handle_t *handle);
void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid);
void sleep_ms(unsigned ms);

#ifdef USE_IOKIT

bool open_usb_device(io_service_t serv, usb_handle_t *handle);
bool open_usb_interface(uint8_t usb_interface, uint8_t usb_alt_interface, usb_handle_t *handle);
void close_usb_interface(usb_handle_t *handle);
void close_usb_device(usb_handle_t *handle);

#endif

#endif /* _GASTER_H */
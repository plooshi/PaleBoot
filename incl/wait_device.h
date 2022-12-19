#ifndef _WAIT_DEVICE_H
#define _WAIT_DEVICE_H

#include "usb.h"

int wait_usb_handles(usb_handle_t **found_targets, int targets[][2], unsigned int target_count);

#endif /* _WAIT_DEVICE_H */
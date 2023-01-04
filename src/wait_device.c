#include <gaster.h>
#include <wait_device.h>
#include <stdio.h>
#include <logging.h>

#ifdef USE_LIBUSB
int wait_usb_handles(usb_handle_t **found_targets, int targets[][2], unsigned int target_count) {
    libusb_device **list;
    libusb_context *context;
    usb_handle_t found_targets_big[1024];
    unsigned int found_count = 0;

    if (libusb_init(&context) == LIBUSB_SUCCESS) {
        log_debug("Waiting for devices...\n");

        for (;;) {
            ssize_t device_cnt = libusb_get_device_list(context, &list);

            for (int i = 0; i < device_cnt; i++) {
                libusb_device *device = list[i];
                struct libusb_device_descriptor desc = {0};

                libusb_get_device_descriptor(device, &desc);

                for (int x = 0; x < target_count; x++) {
                    if (targets[x][0] == desc.idVendor && targets[x][1] == desc.idProduct) {
                        init_usb_handle(&found_targets_big[found_count], desc.idVendor, desc.idProduct);
                        found_count++;
                    }
                }
            }

            libusb_free_device_list(list, device_cnt);

            if (found_count > 0) {
                break;
            }
        }

        libusb_exit(context);
    }

    usb_handle_t found_targets_out[found_count];

    for (int i = 0; i < found_count; i++) {
        found_targets_out[i] = found_targets_big[i];
    }
    
    *found_targets = found_targets_out;
    return found_count;
}
#else
int wait_usb_handles(usb_handle_t **found_targets, int targets[][2], unsigned int target_count) {
    CFMutableDictionaryRef matching_dict;
    io_iterator_t iter;
    io_service_t serv;
    usb_handle_t found_targets_big[1024];
    unsigned int found_count = 0;

    log_debug("Waiting for devices...\n");
    
    while((matching_dict = IOServiceMatching(kIOUSBDeviceClassName)) != NULL) {
        if(IOServiceGetMatchingServices(0, matching_dict, &iter) == kIOReturnSuccess) {
            while((serv = IOIteratorNext(iter)) != IO_OBJECT_NULL) {
                usb_handle_t *handle;

                if(open_usb_device(serv, handle)) {
                    if(open_usb_interface(0, 0, handle)) {
                        unsigned short vid, pid;
                        (*handle->device)->GetDeviceVendor(handle->device, &vid);
                        (*handle->device)->GetDeviceProduct(handle->device, &pid);

                        for (int x = 0; x < target_count; x++) {
                            if (targets[x][0] == vid && targets[x][1] == pid) {
                                found_targets_big[found_count] = handle;
                                found_count++;
                            }
                        }
                    }
                    close_usb_interface(handle);
                }
                close_usb_device(serv);
            }

            IOObjectRelease(iter);
        }

        if (found_count > 0) {
            break;
        }
    }

    usb_handle_t found_targets_out[found_count];

    for (int i = 0; i < found_count; i++) {
        found_targets_out[i] = found_targets_big[i];
    }
    
    *found_targets = found_targets_out;
    return found_count;
}
#endif
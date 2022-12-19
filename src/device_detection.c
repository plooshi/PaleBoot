// built in
#include <wait_device.h>
#include <device_detection.h>

const char *get_device_mode() {
    usb_handle_t *found_targets;
    int targets[][2] = {
        // normal mode
        {0x05ac, 0x12a8},
        {0x05ac, 0x12aa},
        {0x05ac, 0x12ab},

        // recovery mode
        {0x05ac, 0x1281},

        // dfu mode
        {0x05ac, 0x1227}
    };
    int found_count = wait_usb_handles(found_targets, targets, sizeof(targets) / sizeof(targets[0]));
    if (found_count > 1) {
        return "too_many";
    }
    usb_handle_t handle = found_targets[0];
    unsigned short pid = (&handle)->pid;

    if (pid == 0x12a8 || pid == 0x12aa || pid == 0x12ab) {
        return "normal";
    } else if (pid == 0x1281) {
        return "recovery";
    } else if (pid == 0x1227) {
        return "dfu";
    }

    return "how";
}
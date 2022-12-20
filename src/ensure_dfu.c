// built in
#include <device_detection.h>
#include <get_udid.h>
#include <utils.h>
#include <ensure_dfu.h>
#include <usb.h>
#include <wait_device.h>
#include <gaster.h>

// deps
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

// std c library
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

bool enter_recovery(char *udid) {
    idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
    idevice_t device = NULL;
    lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
    lockdownd_client_t client = NULL;
    bool res = true;

    ret = idevice_new(&device, udid);
    if (ret != IDEVICE_E_SUCCESS) {
        printf("Unable to connect to device!\n");
        return false;
    }

    if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new(device, &client, "PaleBoot"))) {
        printf("Could not connect to lockdownd: %s (%d)\n", lockdownd_strerror(ldret), ldret);
        idevice_free(device);
        return false;
    }

    printf("Sending device into recovery mode.\n");
    ldret = lockdownd_enter_recovery(client);
	if (ldret == LOCKDOWN_E_SESSION_INACTIVE) {
		lockdownd_client_free(client);
		client = NULL;
		if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new_with_handshake(device, &client, "PaleBoot"))) {
			printf("Could not connect to lockdownd: %s (%d)\n", lockdownd_strerror(ldret), ldret);
			idevice_free(device);
			return false;
		}
		ldret = lockdownd_enter_recovery(client);
	}
	if (ldret != LOCKDOWN_E_SUCCESS) {
		printf("Failed to enter recovery mode!\n");
		res = false;
	} else {
		printf("Device is now going into recovery mode.\n");
	}

    lockdownd_client_free(client);
	idevice_free(device);

    return res;
}

void step(int time, char *text) {
    for (int i = 0; i < time; i++) {
        printf("\r%s (%d)", text, time - i);
        fflush(stdout);

        sleep(1);
    }
    printf("\r%s (%d)\n", text, 0);
}

bool dfuhelper(unsigned int cpid, char *product_type) {
    char *step_one, *step_two;

    if ((cpid == 0x8010 || cpid == 0x8015) && strstr(product_type, "iPad") != product_type) {
        step_one = "Hold volume down + side button";
    } else {
        step_one = "Hold home + power button";
    }
    printf("Press any key when ready for DFU mode\n");
    getchar();
    step(3, "Get ready");
    step(10, step_one);
    
    if ((cpid == 0x8010 || cpid == 0x8015) && strstr(product_type, "iPad") != product_type) {
        step(10, "Release side button, but keep holding volume down");
    } else {
        step(10, "Release power button, but keep holding home button");
    }

    if (ensure_dfu()) {
        printf("Device successfully entered DFU mode!\n");
        return true;
    } else {
        printf("Device did not enter DFU mode, please run PaleBoot again.\n");
        return false;
    }
}

bool wait_recovery() {
    usb_handle_t *found_targets;
    int targets[][2] = {
        {0x05ac, 0x1281}
    };
    wait_usb_handles(&found_targets, targets, sizeof(targets) / sizeof(targets[0]));

    return ensure_dfu();
}

bool ensure_dfu() {
    const char *device_mode = get_device_mode();

    if (strcmp(device_mode, "too_many") == 0) {
        printf("More than once device detected! Please have only the device you would like to boot plugged in.\n");
        return false;
    } else if (strcmp(device_mode, "normal") == 0) {
        char *udid;

        get_udid(&udid);

        if (strcmp(udid, "error") == 0) {
            printf("Failed to get udid!\n");
            return false;
        }

        if (!enter_recovery(udid)) return false;

        printf("Waiting for device in recovery mode.\n");
        return wait_recovery();
    } else if (strcmp(device_mode, "recovery") == 0) {
        irecv_client_t client = get_client();
        irecv_device_t device = NULL;
	    irecv_devices_get_device_by_client(client, &device);

        return dfuhelper(device->chip_id, (char *)device->product_type);
    } else if (strcmp(device_mode, "dfu") == 0) {
        return true;
    }

    return false;
}
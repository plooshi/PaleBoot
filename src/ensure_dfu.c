// built in
#include <device_detection.h>
#include <get_udid.h>
#include <utils.h>
#include <ensure_dfu.h>
#include <usb.h>
#include <wait_device.h>
#include <gaster.h>
#include <logging.h>

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
        log_error("Unable to connect to device!\n");
        return false;
    }

    if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new(device, &client, "PaleBoot"))) {
        log_error("Could not connect to lockdownd: %s (%d)\n", lockdownd_strerror(ldret), ldret);
        idevice_free(device);
        return false;
    }

    log_debug("Sending device into recovery mode.\n");
    ldret = lockdownd_enter_recovery(client);
	if (ldret == LOCKDOWN_E_SESSION_INACTIVE) {
		lockdownd_client_free(client);
		client = NULL;
		if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new_with_handshake(device, &client, "PaleBoot"))) {
			log_error("Could not connect to lockdownd: %s (%d)\n", lockdownd_strerror(ldret), ldret);
			idevice_free(device);
			return false;
		}
		ldret = lockdownd_enter_recovery(client);
	}
	if (ldret != LOCKDOWN_E_SUCCESS) {
		log_error("Failed to enter recovery mode!\n");
		res = false;
	} else {
		log_info("Device is now going into recovery mode.\n");
	}

    lockdownd_client_free(client);
	idevice_free(device);

    return res;
}

void step(int time, char *text) {
    for (int i = 0; i < time; i++) {
        printf("\r\e[K%s (%d)", text, time - i);
        fflush(stdout);

        sleep(1);
    }
    printf("\r%s (%d)\n", text, 0);
}

bool dfuhelper(unsigned int cpid, char *product_type, bool semi_tethered) {
    char *step_one, *step_two;

    if ((cpid == 0x8010 || cpid == 0x8015) && strstr(product_type, "iPad") != product_type) {
        step_one = "Hold volume down + side button";
    } else {
        step_one = "Hold home + power button";
    }
    log_info("Press any key when ready for DFU mode\n");
    getchar();
    step(3, "Get ready");
    step(10, step_one);
    
    if ((cpid == 0x8010 || cpid == 0x8015) && strstr(product_type, "iPad") != product_type) {
        step(10, "Release side button, but keep holding volume down");
    } else {
        step(10, "Release power button, but keep holding home button");
    }

    if (ensure_dfu(semi_tethered)) {
        log_info("Device successfully entered DFU mode!\n");
        return true;
    } else {
        log_error("Device did not enter DFU mode, please run PaleBoot again.\n");
        return false;
    }
}

bool wait_recovery(bool semi_tethered) {
    usb_handle_t *found_targets;
    int targets[][2] = {
        {0x05ac, 0x1281}
    };
    wait_usb_handles(&found_targets, targets, sizeof(targets) / sizeof(targets[0]));

    return ensure_dfu(semi_tethered);
}

bool ensure_dfu(bool semi_tethered) {
    const char *device_mode = get_device_mode();

    if (strcmp(device_mode, "too_many") == 0) {
        log_error("More than once device detected! Please have only the device you would like to boot plugged in.\n");
        return false;
    } else if (strcmp(device_mode, "normal") == 0) {
        char *udid;

        get_udid(&udid);

        if (strcmp(udid, "error") == 0) {
            log_error("Failed to get udid!\n");
            return false;
        }

        if (!enter_recovery(udid)) return false;

        log_info("Waiting for device in recovery mode.\n");
        return wait_recovery(semi_tethered);
    } else if (strcmp(device_mode, "recovery") == 0) {
        irecv_client_t client = get_client();
        irecv_device_t device = NULL;
	    irecv_devices_get_device_by_client(client, &device);

        unsigned int cpid = device->chip_id;
        char *product = (char *)device->product_type;

        irecv_close(client);

        if (!semi_tethered) {
            if (set_env("auto-boot", "false") != 0) {
                log_warn("Failed to fix auto boot value!\n");
            }
        } else {
            if (set_env("auto-boot", "true") != 0) {
                log_warn("Failed to fix auto boot value!\n");
            }
        }

        return dfuhelper(cpid, product, semi_tethered);
    } else if (strcmp(device_mode, "dfu") == 0) {
        return true;
    }

    return false;
}

bool wait_recovery_no_fix() {
    usb_handle_t *found_targets;
    int targets[][2] = {
        {0x05ac, 0x1281}
    };
    wait_usb_handles(&found_targets, targets, sizeof(targets) / sizeof(targets[0]));

    return ensure_dfu_no_fix();
}

bool ensure_dfu_no_fix() {
    const char *device_mode = get_device_mode();

    if (strcmp(device_mode, "too_many") == 0) {
        log_error("More than once device detected! Please have only the device you would like to boot plugged in.\n");
        return false;
    } else if (strcmp(device_mode, "normal") == 0) {
        char *udid;

        get_udid(&udid);

        if (strcmp(udid, "error") == 0) {
            log_error("Failed to get udid!\n");
            return false;
        }

        if (!enter_recovery(udid)) return false;

        log_info("Waiting for device in recovery mode.\n");
        return wait_recovery_no_fix();
    } else if (strcmp(device_mode, "recovery") == 0) {
        return true;
    } else if (strcmp(device_mode, "dfu") == 0) {
        return true;
    }

    return false;
}
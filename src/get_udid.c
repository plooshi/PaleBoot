#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice-glue/utils.h>
#include <stdlib.h>
#include <string.h>

void get_udid(char *_udid) {
	idevice_t device = NULL;
	idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
    const char* udid = NULL;
    char *real_udid = NULL;

    ret = idevice_new_with_options(&device, udid, IDEVICE_LOOKUP_USBMUX);
    if (ret != IDEVICE_E_SUCCESS) {
		_udid = "error";
	}

    char actual_udid[64] = "";
    idevice_get_udid(device, &real_udid);
    strcpy(actual_udid, real_udid);
    free(real_udid);

	idevice_free(device);
    _udid = actual_udid;
}
#include <gaster.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <libirecovery.h>
#include <unistd.h>

void print_progress_bar(double progress) {
	int i = 0;

	if(progress < 0) {
		return;
	}

	if(progress > 100) {
		progress = 100;
	}

	printf("\r[");

	for(i = 0; i < 50; i++) {
		if(i < progress / 2) {
			printf("=");
		} else {
			printf(" ");
		}
	}

	printf("] %3.1f%%", progress);

	fflush(stdout);

	if(progress == 100) {
		printf("\n");
	}
}

int progress_cb(irecv_client_t client, const irecv_event_t* event) {
	if (event->type == IRECV_PROGRESS) {
		print_progress_bar(event->progress);
	}

	return 0;
}

int send_file(const char *argument) {
    int i = 0;
    int64_t ecid = 0;
    irecv_error_t error = 0;

    irecv_client_t client = NULL;
	for (i = 0; i <= 5; i++) {
		printf("Attempting to connect... \n");

		irecv_error_t err = irecv_open_with_ecid(&client, ecid);
		if (err == IRECV_E_UNSUPPORTED) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return -1;
		}
		else if (err != IRECV_E_SUCCESS)
			sleep(1);
		else
			break;

		if (i == 5) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return -1;
		}
	}

    irecv_device_t device = NULL;
	irecv_devices_get_device_by_client(client, &device);

    irecv_event_subscribe(client, IRECV_PROGRESS, &progress_cb, NULL);
	error = irecv_send_file(client, argument, 1);
	printf("%s\n", irecv_strerror(error));
    return 0;
}

int main() {
    usb_handle_t handle;

    if (!gaster_checkm8(&handle)) {
        printf("Failed to pwn!\n");
        return 1;
    }

    if (!gaster_reset(&handle)) {
        printf("Failed to reset!\n");
        return 1;
    }

    if (send_file("./boot/ibot.img4") == -1) {
        printf("Failed to send iBoot!\n");
        return 1;
    }
     
    return 0;
}
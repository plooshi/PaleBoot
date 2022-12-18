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

irecv_client_t get_client() {
    int i = 0;
    int64_t ecid = 0;

    irecv_client_t client = NULL;
	for (i = 0; i <= 5; i++) {
		irecv_error_t err = irecv_open_with_ecid(&client, ecid);
		if (err == IRECV_E_UNSUPPORTED) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return NULL;
		}
		else if (err != IRECV_E_SUCCESS)
			sleep(1);
		else
			break;

		if (i == 5) {
			fprintf(stderr, "ERROR: %s\n", irecv_strerror(err));
			return NULL;
		}
	}

    return client;
}

int send_file(irecv_client_t client, const char *filename) {

    irecv_event_subscribe(client, IRECV_PROGRESS, &progress_cb, NULL);
	irecv_error_t error = irecv_send_file(client, filename, 1);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int run_command(irecv_client_t client, const char *command) {
    irecv_error_t error = irecv_send_command(client, command);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int main() {
    bool has_ibss = false, has_t8010 = false, has_t8015 = false;
    FILE *fs_file;
    char fs[9] = "";

    if (access("./boot", F_OK) != 0) {
        printf("Couldn't find boot directory!\n");
        return 1;
    }

    if (access("./boot/ibot.img4", F_OK) != 0) {
        printf("Couldn't find iBoot!\n");
        return 1;
    }

    if (access("./boot/iBSS.img4", F_OK) == 0) {
        printf("Found iBSS.\n");
        has_ibss = true;
    }

    if (access("./boot/payload_t8010.bin", F_OK) == 0) {
        printf("Found payload (A10).\n");
        has_t8010 = true;
    }

    if (access("./boot/payload_t8015.bin", F_OK) == 0) {
        printf("Found payload (A11).\n");
        has_t8015 = true;
    }

    if (has_t8010 || has_t8015) {
        if (access("./boot/.fs", F_OK) != 0) {
            printf("Couldn't find boot/.fs!\n");
            return 1;
        } else {
            fs_file = fopen("./boot/.fs", "r");
            fgets(fs, 9, fs_file);
            fclose(fs_file);
        }
    }

    usb_handle_t handle;

    if (!gaster_checkm8(&handle)) {
        printf("Failed to pwn!\n");
        return 1;
    }

    if (!gaster_reset(&handle)) {
        printf("Failed to reset!\n");
        return 1;
    }

    sleep(1);

    irecv_client_t client = get_client();

    irecv_device_t device = NULL;
	irecv_devices_get_device_by_client(client, &device);

    printf("Hi %x (%s)", device->chip_id, device->product_type);

    if (has_ibss) {
        if (send_file(client, "./boot/iBSS.img4") != 0) {
            printf("Failed to send iBSS!\n");
            return 1;
        }
        sleep(3);
        
        client = get_client();
    }

    sleep(1);
    if (send_file(client, "./boot/ibot.img4") != 0) {
        printf("Failed to send iBoot!\n");
        return 1;
    }

    if (has_t8010 || has_t8015) {
        client = get_client();

        sleep(3);
        run_command(client, "dorwx");

        client = get_client();
    }

    if (has_t8010) {
        sleep(2);
        if (send_file(client, "./boot/payload_t8010.bin") != 0) {
            printf("Failed to send payload!\n");
            return 1;
        }

        client = get_client();
    } else if (has_t8015) {
        sleep(2);
        if (send_file(client, "./boot/payload_t8015.bin") != 0) {
            printf("Failed to send payload!\n");
            return 1;
        }

        client = get_client();
    }

    if (has_t8010 || has_t8015) {
        sleep(3);
        if (run_command(client, "go") != 0) {
            printf("Failed to run go!\n");
            return 1;
        };
        sleep(1);
        client = get_client();

        if (run_command(client, "go xargs -v serial=3") != 0) {
            printf("Failed to set boot args!\n");
            return 1;
        };
        sleep(1);
        client = get_client();

        if (run_command(client, "go xfb") != 0) {
            printf("Failed to init framebuffer!\n");
            return 1;
        };
        sleep(1);
        client = get_client();

        char boot_command[18] = "";
        snprintf(boot_command, 18, "go boot %s", fs);
        if (run_command(client, boot_command) != 0) {
            printf("Failed to boot!\n");
            return 1;
        }
        
        client = get_client();
    }
    
    sleep(2);

    int fsboot_ret = run_command(client, "fsboot");
    if (fsboot_ret != 0 && fsboot_ret != -1) {
        printf("Failed to fsboot!\n");
        return 1;
    }

    irecv_close(client);
     
    return 0;
}
#include <gaster.h>
#include <stdio.h>
#include <libirecovery.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define file_exists(file) ((access(file, F_OK)) != -1)
#define startswith(string, substr) (strcmp(strstr(substr, string), string) == 0)

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

int send_file(const char *filename) {
    irecv_client_t client = get_client();
    irecv_event_subscribe(client, IRECV_PROGRESS, &progress_cb, NULL);
	irecv_error_t error = irecv_send_file(client, filename, 1);
    irecv_close(client);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int run_command(const char *command) {
    irecv_client_t client = get_client();
    irecv_error_t error = irecv_send_command(client, command);
    irecv_close(client);
    if (error != 0) {
	    printf("%s\n", irecv_strerror(error));
        return error;
    }
    return 0;
}

int main(int argc, char **argv) {
    FILE *fs_file;
    char fs[9] = "";
    bool semi_tethered = argc > 2 && strcmp(argv[2], "--tethered") != 0;

    if (!file_exists("./boot")) {
        printf("Couldn't find boot directory!\n");
        return 1;
    }

    if (!file_exists("./boot/ibot.img4")) {
        printf("Couldn't find iBoot!\n");
        return 1;
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

    irecv_close(client);

    if (device->chip_id != 0x8010 && device->chip_id != 0x8015) {
        if (!file_exists("./boot/iBSS.img4")) {
            printf("Could not find iBSS!\n");
            return 1;
        } else {
            if (send_file("./boot/iBSS.img4") != 0) {
                printf("Failed to send iBSS!\n");
                return 1;
            }

            sleep(3);
        }
    }

    bool do_hb_patch = semi_tethered && startswith("iPhone9,", device->product_type) || (startswith("iPhone10,", device->product_type) && 
        !startswith("iPhone10,3", device->product_type) && !startswith("iPhone10,6", device->product_type));

    sleep(1);
    if (send_file("./boot/ibot.img4") != 0) {
        printf("Failed to send iBoot!\n");
        return 1;
    } else {
        if (!do_hb_patch || semi_tethered) {
            printf("Successfully booted device!\n");
            return 0;
        }
    }

    char payload_path[25] = "not.found";
    if (do_hb_patch) {
        if (file_exists("./boot/payload_t8010.bin")) {
            strcpy(payload_path, "./boot/payload_t8010.bin");
        } else if (file_exists("./boot/payload_t8015.bin")) {
            strcpy(payload_path, "./boot/payload_t8015.bin");
        }
        
        if (strcmp(payload_path, "not.found")) {
            printf("PaleBoot detected your device needs payload, but we could not find it in the boot folder.\n");
            printf("Please copy the correct payload for your device from other/payload in your palera1n folder to the boot folder.\n");
            printf("If using tethered, please add --tethered to the end of the command\n");
            return 1;
        }

        if (!file_exists("./boot/.fs")) {
            printf("Couldn't find boot/.fs!\n");
            return 1;
        } else {
            fs_file = fopen("./boot/.fs", "r");
            fgets(fs, 9, fs_file);
            fclose(fs_file);
        }

        sleep(3);

        if (run_command("dorwx") != 0) {
            printf("Failed to run dorwx!\n");
            return 1;
        }

        sleep(2);
        
        if (send_file(payload_path) != 0) {
            printf("Failed to send payload!\n");
            return 1;
        }

        sleep(3);

        if (run_command("go") != 0) {
            printf("Failed to run boot payload!\n");
            return 1;
        };

        sleep(1);

        if (run_command("go xargs -v serial=3") != 0) {
            printf("Failed to set boot args!\n");
            return 1;
        }

        sleep(1);

        if (run_command("go xfb") != 0) {
            printf("Failed to init framebuffer!\n");
            return 1;
        }

        sleep(1);

        char boot_command[18] = "";
        snprintf(boot_command, 18, "go boot %s", fs);
        if (run_command(boot_command) != 0) {
            printf("Failed to boot!\n");
            return 1;
        } else {
            printf("Successfully booted device!\n");
            return 0;
        }
    }
    
    sleep(2);

    if (!semi_tethered) {
        if (run_command("fsboot") != 0) {
            printf("Failed to fsboot!\n");
            return 1;
        } else {
            printf("Successfully booted device!\n");
            return 0;
        }
    }
     
    return 0;
}
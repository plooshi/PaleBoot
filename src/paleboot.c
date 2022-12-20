// built in
#include <gaster.h>
#include <ensure_dfu.h>

// deps
#include <libirecovery.h>
#include <utils.h>

// std c library
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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

    ensure_dfu();

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

    bool do_hb_patch = semi_tethered && strstr("iPhone9,", device->product_type) == device->product_type || 
        strstr("iPhone10,", device->product_type) == device->product_type;

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
            printf("Failed to boot payload!\n");
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
// built in
#include <gaster.h>
#include <ensure_dfu.h>
#include <download.h>
#include <device_detection.h>

// deps
#include <libirecovery.h>
#include <utils.h>

// std c library
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main() {
    FILE *fs_file;
    char fs[9] = "";
    bool semi_tethered;

    ensure_dfu_no_fix();

    irecv_client_t client = get_client();

    irecv_device_t device = NULL;
	irecv_devices_get_device_by_client(client, &device);

    irecv_close(client);

    char boot_path[18], ibot_path[28], ibss_path[28];
    char semi_path[24], tether_path[26];
    
    sprintf(boot_path, "./boot-%s", device->product_type);

    sprintf(ibot_path, "%s/ibot.img4", boot_path);
    sprintf(ibss_path, "%s/iBSS.img4", boot_path);
    sprintf(semi_path, "%s/.semi", boot_path);
    sprintf(tether_path, "%s/.tether", boot_path);

    if (!file_exists(boot_path)) {
        printf("Couldn't find boot directory!\n");
        return 1;
    }

    if (!file_exists(ibot_path)) {
        printf("Couldn't find iBoot!\n");
        return 1;
    }

    if (file_exists(semi_path)) {
        semi_tethered = true;
    } else if (file_exists(tether_path)) {
        semi_tethered = false;
    } else {
        printf("Doing first-run setup, please wait...\n"); 

        FILE* iboot_fp = fopen(ibot_path, "rb");

        char *iboot_data;
        read_all(&iboot_data, iboot_fp);

        semi_tethered = strstr(iboot_data, "rd=disk");

        if (semi_tethered) {
            close(open(semi_path, O_RDWR | O_CREAT, 664));
        } else {
            close(open(tether_path, O_RDWR | O_CREAT, 664));
        }
    }

    ensure_dfu(semi_tethered);

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

    if (device->chip_id != 0x8010 && device->chip_id != 0x8015) {
        if (!file_exists(ibss_path)) {
            printf("Could not find iBSS!\n");
            return 1;
        } else {
            if (send_file(ibss_path) != 0) {
                printf("Failed to send iBSS!\n");
                return 1;
            }

            sleep(3);
        }
    }

    bool do_hb_patch = semi_tethered && strstr("iPhone9,", device->product_type) == device->product_type || 
        strstr("iPhone10,", device->product_type) == device->product_type;

    sleep(1);
    if (send_file(ibot_path) != 0) {
        printf("Failed to send iBoot!\n");
        return 1;
    } else {
        if (semi_tethered) {
            printf("Successfully booted device!\n");
            return 0;
        }
    }

    if (do_hb_patch) {
        char payload_path[36] = "";

        if (device->chip_id == 0x8010) {
            sprintf(payload_path, "%s/payload_t8010.bin", boot_path);
            if (!file_exists(payload_path)) {
                download_file("https://github.com/palera1n/palera1n/raw/main/other/payload/payload_t8010.bin", payload_path);
            }
        } else {
            sprintf(payload_path, "%s/payload_t8010.bin", boot_path);
            if (!file_exists(payload_path)) {
                download_file("https://github.com/palera1n/palera1n/raw/main/other/payload/payload_t8015.bin", payload_path);
            }
        }

        char fs_path[22];

        sprintf(fs_path, "%s/.fs", boot_path);

        if (!file_exists(fs_path)) {
            printf("Couldn't find %s!\n", fs_path);
            return 1;
        } else {
            fs_file = fopen(fs_path, "r");
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